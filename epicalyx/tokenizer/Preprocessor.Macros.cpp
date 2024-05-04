#include "Preprocessor.h"
#include "Identifier.h"


namespace epi {

Preprocessor::Definition::value_t Preprocessor::Definition::Parse(
  cotyl::vector<cotyl::CString>&& args,
  bool variadic,
  cotyl::CString&& value
) {
  value_t result{};
  auto valstream = SString(value.view());
  cotyl::StringStream current_val{};
  char c;

  // any time we encounter a macro argument, we KNOW that current_val will be non-empty
  // so we can append a new string to the result
  while (valstream.Peek(c, 0)) {
    if (detail::is_valid_ident_start(c)) {
      auto ident = detail::get_identifier(valstream);
      if (variadic && ident.streq("__VA_ARGS__")) {
        // variadic argument, emplace current intermediate text and reset
        result.emplace_back(current_val.cfinalize());
        current_val.clear();
        result.emplace_back(-1);
      }
      else {
        auto arg_idx = std::find(args.begin(), args.end(), ident);
        if (arg_idx != args.end()) {
          // argument id, emplace current intermediate text and reset
          result.emplace_back(current_val.cfinalize());
          current_val.clear();
          result.emplace_back((i32)(arg_idx - args.begin()));
        }
        else {
          // just append the identifier
          current_val << ident;
        }
      }
    }
    else {
      current_val << valstream.Get();
    }
  }

  if (!current_val.empty()) {
    result.emplace_back(current_val.cfinalize());
  }
  return result;
}

const cotyl::CString Preprocessor::MacroStream::InitialStream = cotyl::CString{" "};

char Preprocessor::MacroStream::GetNew() {
  if (eos) {
    throw cotyl::EOSError();
  }
  else if (current_stream.EOS()) {
    if (++current_index < def.value.size()) {
      swl::visit(
        swl::overloaded{
          [&](const cotyl::CString& seg) {
            current_stream = SString(seg.view());
          },
          [&](const i32 seg) {
            if (seg == -1) {
              current_stream = SString(va_args.view());
            }
            else {
              current_stream = SString(arguments[seg].view());
            }
          },
          // exhaustive variant access
          [](const auto& invalid) { static_assert(!sizeof(invalid)); }
        },
        def.value[current_index]
      );
    }
    else {
      eos = true;
    }
    return ' ';  // end every stream in whitespace
  }
  return current_stream.Get();
}

bool Preprocessor::MacroStream::IsEOS() {
  return eos;
}

void Preprocessor::MacroStream::PrintLoc(std::ostream& out) const {
  // todo
}

bool Preprocessor::MacroStream::ExpandingArgument() const {
  if (eos) return false;
  return swl::holds_alternative<i32>(def.value[current_index]);
}

cotyl::CString Preprocessor::GetMacroArgumentValue(bool variadic) {
  cotyl::StringStream value{};
  SkipBlanks();

  unsigned paren_count = 0;
  while (true) {
    // a comma or a brace is a part of a string if and only if the pre_processor_queue is not empty
    const char next = NextCharacter();
    if (next == ',' && !variadic && paren_count == 0) {
      // next argument
      return value.cfinalize();
    }
    else if (next == ')') {
      // if paren_count is 0 there are no more arguments and this argument is done
      if (paren_count == 0) return value.cfinalize();
      else {
        // otherwise, track the parenthesis
        paren_count--;
        EatNextCharacter(')');
        value << ')';
      }
    }
    else if (next == '(') {
      // track parentheses
      EatNextCharacter('(');
      paren_count++;
      value << '(';
    }
    else {
      // process macro argument value, as they are unfolded properly in the definition anyway
      value << GetNextCharacter();
    }
  }
}

void Preprocessor::PushMacro(cotyl::CString&& name, const Definition& definition) {
  if (definition.value.empty()) {
    // no use pushing an empty macro
    return;
  }

  if (definition.arguments.has_value()) {
    const auto& arguments = definition.arguments.value();
    cotyl::vector<cotyl::CString> arg_values{};
    cotyl::CString va_args{};

    // when parsing macro usage, newlines can be skipped just fine
    SkipBlanks();
    EatNextCharacter('(');
    SkipBlanks();
    for (int i = 0; i < arguments.count; i++) {
      auto arg_val = GetMacroArgumentValue(false);
      ReplaceNewlines(arg_val);
      arg_values.emplace_back(std::move(arg_val));

      if ((i != (arguments.count - 1))) {
        // not last element
        EatNextCharacter(',');
        SkipBlanks();
      }
    }
    if (arguments.variadic && NextCharacter() != ')') {
      if (arguments.count) {
        // no , if variadic arguments are first arguments
        EatNextCharacter(',');
        SkipBlanks();
      }
      va_args = GetMacroArgumentValue(true);
      ReplaceNewlines(va_args);
    }
    EatNextCharacter(')');

    // definition value has already been cleaned on #define
    macro_stack.emplace_back(std::move(name), definition, std::move(arg_values), std::move(va_args));
  }
  else {
    // definition value has already been cleaned on #define
    macro_stack.emplace_back(std::move(name), definition);
  }
}

}