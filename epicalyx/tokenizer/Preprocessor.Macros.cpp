#include "Preprocessor.h"
#include "Identifier.h"
#include "Escape.h"
#include "CustomAssert.h"


namespace epi {

Preprocessor::Definition::value_t Preprocessor::Definition::Parse(
  cotyl::vector<cotyl::CString>&& args,
  bool variadic,
  cotyl::CString&& value
) {
  value_t result{};
  auto valstream = SString{std::move(value)};
  cotyl::StringStream current_val{};
  char c;

  // expand an encountered argument
  // set to false when we encounter a ## operator
  bool concat_next = false;

  auto end_segment = [&] {
    if (!current_val.empty()) {
      result.emplace_back(current_val.cfinalize());
      current_val.clear();

      // reset concatenation state
      concat_next = false;
    }
  };

  auto get_argument = [&](const cotyl::CString& ident) -> std::optional<i32> {
    // variadic argument
    if (variadic && ident.streq("__VA_ARGS__")) return -1;
    
    // non-variadic argument
    auto arg_it = std::find(args.begin(), args.end(), ident);
    if (arg_it != args.end()) return (i32)(arg_it - args.begin());

    // identifier is not an argument name
    return {};
  };

  while (valstream.Peek(c, 0)) {
    if (c == '\"') {
      // strings in macro values must be taken literally, as to 
      // not expand macro arguments or #/## operators within them
      current_val << valstream.Expect(c);
      for (char k = valstream.Get(); k != '\"'; k = valstream.Get()) {
        current_val << k;
        if (k == '\\') {
          // escape sequence
          current_val << valstream.Get();
        }
      }

      // end quote (this was already taken from the stream)
      current_val << '\"';
    }
    else if (std::isspace(c)) {
      end_segment();
      valstream.SkipWhile(std::isspace);
    }
    else if (c == '#') {
      end_segment();
      valstream.Expect(c);

      if (valstream.EatIf('#')) {
        if (result.empty()) {
          throw PreprocessorError("Unexpected '##' operator at the start of macro definition");
        }

        // don't expand previous argument
        if (swl::holds_alternative<Argument>(result.back())) {
          swl::get<Argument>(result.back()).concat_next = true;
        }

        // concat next section
        concat_next = true;
      }
      else {
        // get consequent macro argument value
        valstream.SkipWhile(std::isspace);
        auto ident = detail::get_identifier(valstream);
        auto arg = get_argument(ident);
        if (!arg.has_value()) {
          throw PreprocessorError("Expected macro argument after # operator");
        }
        // argument will not be expanded
        result.emplace_back(Hash{arg.value()});

        // reset concatenation state
        concat_next = false;
      }
    }
    else if (detail::is_valid_ident_start(c)) {
      auto ident = detail::get_identifier(valstream);
      auto arg = get_argument(ident);
      if (arg.has_value()) {
        end_segment();

        // expand argument only if it is not concatenated
        result.emplace_back(Argument{.arg_index = arg.value(), .expand = !concat_next});

        // reset concatenation state
        concat_next = false;
      }
      else {
        // normal identifier, just append it
        current_val << ident;
      }
    }
    else if (detail::is_valid_ident_char(c)) {
      // might be an integer, which has a qualifier at the end, like
      // 201112L. We don't want to view L as a macro argument value
      while (valstream.PredicateAfter(0, detail::is_valid_ident_char)) {
        current_val << valstream.Get();
      }
    }
    else {
      current_val << valstream.Expect(c);
    }
  }

  // end potential last segment
  end_segment();
  if (concat_next) {
    throw PreprocessorError("Unexpected '##' operator at the end of macro definition");
  }
  return result;
}

template<typename T>
static cotyl::CString quoted(T&& string) {
  auto stream = cotyl::StringStream{};
  stream << '\"' << string << '\"';
  return stream.cfinalize();
}

void Preprocessor::MacroStream::PrintLoc(std::ostream& out) const {
  out << "In expansion of macro " << name.c_str() << std::endl;
  this->SString::PrintLoc(out);
}

std::pair<cotyl::CString, char> Preprocessor::GetMacroArgumentValue(bool variadic) {
  cotyl::Assert(!block_macro_expansion, "Invalid macro state in preprocessor");  
  cotyl::StringStream value{};
  char separator = '\0';
  SkipBlanks();

  unsigned paren_count = 0;
  block_macro_expansion = true;
  while (true) {
    // a comma or a brace is a part of a string if and only if the pre_processor_queue is not empty
    auto next = GetNextProcessed();
    if (next.empty()) continue;
    if (next.size() == 1) {
      // single characters may be parseable
      char c = next[0];
      if (c == ',' && !variadic && paren_count == 0) {
        // next argument
        separator = c;
        break;
      }
      else if (c == ')') {
        // if paren_count is 0 there are no more arguments and this argument is done
        if (paren_count == 0) {
          separator = c;
          break;
        }
        else {
          // otherwise, track the parenthesis
          paren_count--;
          value << ')';
        }
      }
      else if (c == '(') {
        // track parentheses
        paren_count++;
        value << '(';
      }
      else {
        // process macro argument value, as they are unfolded properly in the definition anyway
        value << c;
      }
    }
    else {
      // identifier, string, whitespace...
      value << next;
    }
  }

  block_macro_expansion = false;
  return {cotyl::CString{value.trim_view()}, separator};
}

void Preprocessor::ExpandArgumentTo(cotyl::StringStream& dest, const cotyl::CString& arg) {
  cotyl::Assert(!block_macro_expansion);
  
  // we may be parsing an expression or expanding a macro
  auto old_state = std::move(state);
  state = {};
  state.expression = {arg.view()};

  while (!InternalIsEOS()) {
    auto next = GetNextProcessed();
    dest << std::move(next);
    ClearEmptyStreams();
  }

  // restore state
  EndExpression(std::move(old_state));
}

cotyl::CString Preprocessor::ExpandMacro(const Definition& def, cotyl::vector<cotyl::CString>&& args, cotyl::CString&& va_args) {
  cotyl::Assert(args.size() == def.arguments.value_or(Definition::Arguments{}).count);
  cotyl::StringStream value{};

  auto arg_value = [&](i32 arg_index) -> const cotyl::CString& {
    if (arg_index == -1) {
      return va_args;
    }
    return args[arg_index];
  };

  for (int i = 0; i < def.value.size(); i++) {
    bool concat_next = false;

    swl::visit(
      swl::overloaded{
        [&](const cotyl::CString& seg) {
          value << seg;
        },
        [&](const Definition::Argument& seg) {
          if (seg.concat_next || !seg.expand) {
            // don't expand when concatenated
            concat_next = seg.concat_next;
            value << arg_value(seg.arg_index);
          }
          else {
            ExpandArgumentTo(value, arg_value(seg.arg_index));
          }
        },
        [&](const Definition::Hash& hash) {
          // stringify UNEXPANDED argument value
          cotyl::StringStream escaped{};
          auto argvalue = SString{arg_value(hash.arg_index).view()};
          while (!argvalue.EOS()) {
            char c = argvalue.Get();
            if (c == '\\') cotyl::Unescape(escaped, argvalue);
            else escaped << c;
          }

          // re-escape argument value and turn into string
          // store in the holder since it needs to be kept alive until
          // we finish parsing this string
          value << quoted(cotyl::Escape(escaped.cfinalize().c_str()));
        },
        // exhaustive variant access
        [](const auto& invalid) { static_assert(!sizeof(invalid)); }
      },
      def.value[i]
    );

    if (!concat_next && i != def.value.size() - 1) {
      // insert whitespace after arguments
      value << ' ';
    }
  }

  return value.cfinalize();
}

void Preprocessor::PushMacro(cotyl::CString&& name, const Definition& definition) {
  if (definition.value.empty()) {
    // no use pushing an empty macro
    return;
  }

  cotyl::vector<cotyl::CString> arg_values{};
  cotyl::CString va_args{};
  if (definition.arguments.has_value()) {
    const auto& arguments = definition.arguments.value();

    // when parsing macro usage, newlines can be skipped just fine
    SkipBlanks();
    EatNextCharacter('(');
    SkipBlanks();
    char sep = '\0';
    for (int i = 0; i < arguments.count; i++) {
      auto [arg_val, _sep] = GetMacroArgumentValue(false);
      sep = _sep;
      ReplaceNewlines(arg_val);
      arg_values.emplace_back(std::move(arg_val));

      if ((i != (arguments.count - 1))) {
        // not last element
        if (sep != ',') {
          throw PreprocessorError("Too few arguments for functional macro call");
        }
        SkipBlanks();
      }
    }

    if (arguments.variadic && sep != ')') {
      if (arguments.count) {
        // no , if variadic arguments are first arguments
        SkipBlanks();
      }
      auto [_va_args, _sep] = GetMacroArgumentValue(true);
      va_args = std::move(_va_args);
      sep = _sep;
      ReplaceNewlines(va_args);
    }
    
    if (sep == '\0') {
      // no arguments were fetched
      EatNextCharacter(')');
    }
    else if (sep != ')') {
      throw PreprocessorError("Expected ')' to terminate function macro call");
    }
  }

  // expand and push macro
  state.macro_stack.emplace_back(
    std::move(name), 
    ExpandMacro(
      definition, std::move(arg_values), std::move(va_args)
    )
  );
}

}