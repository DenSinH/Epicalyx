#include "Preprocessor.h"
#include "Identifier.h"
#include "CustomAssert.h"

#include <cctype>

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

  auto end_segment = [&] {
    if (!current_val.empty()) {
      auto current = cotyl::CString{current_val.trim_view()};
      if (!current.empty()) result.emplace_back(Definition::Literal{std::move(current)});
      current_val.clear();
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
      valstream.SkipWhile(isspace);
    }
    else if (c == '#') {
      end_segment();
      valstream.Expect(c);

      if (valstream.EatIf('#')) {
        if (result.empty()) {
          throw PreprocessorError("Unexpected '##' operator at the start of macro definition");
        }

        // concat previous segment
        if (swl::holds_alternative<Hash>(result.back().seg)) {
          throw PreprocessorError("Concatenating a stringified macro will not produce a valid preprocessing token"); 
        }
        result.back().concat_next = true;
      }
      else {
        // get consequent macro argument value
        valstream.SkipWhile(isspace);
        auto ident = detail::get_identifier(valstream);
        auto arg = get_argument(ident);
        if (!arg.has_value()) {
          throw PreprocessorError("Expected macro argument after # operator");
        }
        // argument will not be expanded
        result.emplace_back(Hash{arg.value()});
      }
    }
    else if (detail::is_valid_ident_start(c)) {
      auto ident = detail::get_identifier(valstream);
      auto arg = get_argument(ident);
      if (arg.has_value()) {
        end_segment();

        // expand argument only if it is not concatenated
        result.emplace_back(Argument{.arg_index = arg.value()});
      }
      else if (ident == name) {
        end_segment();
        result.emplace_back(Definition::ThisName{});
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

  // check whether we ended with a ## operator
  if (!result.empty() && result.back().concat_next) {
    throw PreprocessorError("Unexpected '##' operator at the end of macro definition");
  }
  return result;
}

void Preprocessor::MacroStream::PrintLoc(std::ostream& out) const {
  out << "In expansion of macro " << name.c_str() << std::endl;
  value.at(current).value.PrintLoc(out);
}

char Preprocessor::MacroStream::GetNew() {
  if (value[current].value.EOS()) {
    if (++current == value.size()) {
      throw cotyl::EOSError();
    }
    return ' ';
  }
  return value[current].value.Get();
}

bool Preprocessor::MacroStream::IsEOS() {
  if (current < value.size() - 1) {
    return false;
  }
  return value.back().value.EOS();
}

std::pair<cotyl::CString, char> Preprocessor::GetMacroArgumentValue(bool variadic) {
  cotyl::StringStream value{};
  char separator = '\0';
  SkipBlanks();

  unsigned paren_count = 0;
  while (true) {
    // a comma or a brace is a part of a string if and only if the pre_processor_queue is not empty
    auto next = GetNextChunk(false);
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

  return {cotyl::CString{value.trim_view()}, separator};
}

cotyl::CString Preprocessor::ExpandArgument(const cotyl::CString& arg) {
  // we may be parsing an expression or expanding a macro
  auto old_state = StartExpression(arg);

  cotyl::StringStream dest{};
  while (!InternalIsEOS()) {
    dest << GetNextChunk();
    ClearEmptyStreams();
  }

  // restore state
  EndExpression(std::move(old_state));
  return dest.cfinalize();
}

cotyl::vector<Preprocessor::MacroStream::Segment> Preprocessor::ExpandMacro(const Definition& def, cotyl::vector<cotyl::CString>&& args, cotyl::CString&& va_args) {
  cotyl::Assert(args.size() == def.arguments.value_or(Definition::Arguments{}).count);
  cotyl::vector<Preprocessor::MacroStream::Segment> result{};

  auto arg_value = [&](i32 arg_index) -> const cotyl::CString& {
    if (arg_index == -1) {
      return va_args;
    }
    return args[arg_index];
  };

  for (int i = 0; i < def.value.size(); i++) {
    const auto& seg = def.value[i];
    if (seg.concat_next) {
      cotyl::StringStream concatenated{};
      do {
        swl::visit(
          swl::overloaded{
            [&](const Definition::Argument& arg) {
              concatenated << arg_value(arg.arg_index);
            },
            [&](const Definition::Literal& lit) {
              concatenated << lit.value;
            },
            [&](const Definition::ThisName&) {
              concatenated << def.name;
            },
            [](const Definition::Hash&) {
              throw cotyl::UnreachableException();
            },
            // exhaustive variant access
            [](const auto& invalid) { static_assert(!sizeof(invalid)); }
          },
          def.value[i].seg
        );

        // we will not read past the end of the array,
        // this is checked upon definition
      } while (def.value[i++].concat_next);
      // don't skip past the last attached
      i--;
      result.emplace_back(ExpandArgument(concatenated.cfinalize()), true);
    }
    else {
      swl::visit(
        swl::overloaded{
          [&](const Definition::Literal& lit) {
            // unexpanded
            result.emplace_back(cotyl::CString{lit.value}, false);
          },
          [&](const Definition::Argument& arg) {
            result.emplace_back(ExpandArgument(arg_value(arg.arg_index)), true);
          },
          [&](const Definition::ThisName&) {
            // unexpanded name
            result.emplace_back(cotyl::CString{def.name}, true);
          },
          [&](const Definition::Hash& hash) {
            // stringify UNEXPANDED argument value
            cotyl::StringStream value{};
            value << '\"';
            auto argvalue = SString{arg_value(hash.arg_index).view()};
            while (!argvalue.EOS()) {
              char c = argvalue.Get();
              if (c == '\\') value << c << argvalue.Get();
              else if (c == '\"') value << '\\' << c;
              else value << c;
            }
            value << '\"';
            result.emplace_back(value.cfinalize(), true);
          },
          // exhaustive variant access
          [](const auto& invalid) { static_assert(!sizeof(invalid)); }
        },
        seg.seg
      );
    }
  }

  return result;
}

bool Preprocessor::IsNestedExpansion(const cotyl::CString& name) const {
  return std::any_of(
    state.macro_stack.begin(), state.macro_stack.end(),
    [&](const auto& macro) {
      return macro.name == name;
    }
  );
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
      cotyl::CString arg_val;
      std::tie(arg_val, sep) = GetMacroArgumentValue(false);
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
      std::tie(va_args, sep) = GetMacroArgumentValue(true);
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