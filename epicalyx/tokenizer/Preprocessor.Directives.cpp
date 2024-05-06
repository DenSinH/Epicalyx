#include "Preprocessor.h"
#include "CustomAssert.h"
#include "Identifier.h"

#include "ast/Expression.h"

#include <filesystem>


namespace epi {

ast::pExpr Preprocessor::ResolveIdentifier(cotyl::CString&& name) const {
  // this MUST be an identifier that is not a defined macro
  // for example, the "defined" preprocessing directive
  if (name.streq("defined")) {
    int parens;
    block_macro_expansion = true;
    for (parens = 0; this_tokenizer.EatIf(TokenType::LParen); parens++);
    auto ident = this_tokenizer.Expect(TokenType::Identifier);
    auto macro = std::move(ident.get<IdentifierToken>().name);
    bool has_macro = (StandardDefinitions.contains(macro) || definitions.contains(macro));
    auto expr = std::make_unique<ast::NumericalConstantNode<i32>>(has_macro ? 1 : 0);
    for (; parens > 0; parens--) {
      this_tokenizer.Eat(TokenType::RParen);
    }
    block_macro_expansion = false;
    return std::move(expr);
  }
  // unresolved identifier, i.e. unexpanded macro evaluates to false
  return std::make_unique<ast::NumericalConstantNode<i32>>(0);
}

void Preprocessor::EndExpression(State&& old_state) {
  // we expect the string (expression) to be fully parsed
  cotyl::Assert((ClearEmptyStreams(), state.macro_stack.empty()), "Found unexpanded macros after expression");
  cotyl::Assert(state.pre_processor_queue.empty());
  cotyl::Assert(state.expression.value().EOS());
  state = std::move(old_state);
}

i64 Preprocessor::EatConstexpr() {
  auto line = FetchLine();

  State old_state = std::move(state);
  state = {};
  state.expression = {line.view()};
  auto result = EConstexpr();
  EndExpression(std::move(old_state));

  // we fetched the whole line, so we are always at a new line after this
  is_newline = true;
  return result;
}

std::string Preprocessor::FindFile(const cotyl::CString& name, bool system) {
  if (!system) {
    // first search in current file's directory
    const auto& current = file_stack.back().name;
    auto full_path = std::filesystem::canonical(current);
    auto parent = full_path.parent_path();
    auto search_path = parent / name.c_str();
    if (std::filesystem::is_regular_file(search_path)) {
      return search_path.string();
    }
  }
  throw cotyl::FormatExcept<PreprocessorError>("Failed to find file %s", name.c_str());
}

void Preprocessor::Include() {
  // parse include directive
  // the file name may be constructed from macro expansion
  // we parse it by creating a tokenizer and requesting a string constant
  auto line = FetchLine();

  State old_state = std::move(state);
  state = {};
  state.expression = {line.view()};
  cotyl::CString filename;
  // skip whitespace
  this_tokenizer.SkipBlanks();
  auto delimiter = ForcePeek();
  bool system_include = *delimiter == '<';
  filename = this_tokenizer.ReadString();
  file_stack.emplace_back(FindFile(filename, system_include));
  
  // same as in EatConstexpr()
  // we expect the bottom string (expression) to be fully parsed
  this_tokenizer.SkipBlanks();
  EndExpression(std::move(old_state));

  // we fetched the whole line, we are at a new line at this point
  // the status might not reflect it though, as we parsed the filename expression
  is_newline = true;
}

void Preprocessor::PreprocessorDirective() {
  // newline state is updated in EatNextCharacter
  EatNextCharacter('#');

  SkipBlanks();
  if (is_newline) {
    // empty line after # character is allowed
    return;
  }

  // read identifier
  const cotyl::CString pp_token = detail::get_identifier(CurrentStream());
  bool enabled = Enabled();
  
  if (pp_token.streq("if") || pp_token.streq("ifdef") || pp_token.streq("ifndef")) {
    SkipBlanks(false);
    if (!enabled) {
      FetchLine();
      if_group_stack.emplace_back(IfGroup::If(false));
    }
    else {
      if (pp_token.streq("if")) {
        if_group_stack.emplace_back(IfGroup::If(EatConstexpr() != 0));
        // newline eaten in EatConstexpr() >> FetchLine()
      }
      else {
        auto identifier = detail::get_identifier(CurrentStream());
        bool has_def = definitions.contains(identifier) || StandardDefinitions.contains(identifier);
        if (pp_token.streq("ifdef")) {
          if_group_stack.emplace_back(IfGroup::If(has_def));
        }
        else {
          cotyl::Assert(pp_token.streq("ifndef"));
          if_group_stack.emplace_back(IfGroup::If(!has_def));
        }
        EatNewline();
      }
    }
  }
  else if (pp_token.streq("elif") || pp_token.streq("elifdef") || pp_token.streq("elifndef")) {
    if (if_group_stack.empty()) {
      throw PreprocessorError("Unexpected elif group");
    }
    SkipBlanks(false);
    if (enabled) {
      // don't care about condition value, is false anyway
      FetchLine();
    }
    else {
      bool parent_disabled = if_group_stack.size() > 1 && std::any_of(
          if_group_stack.begin(),
          std::prev(if_group_stack.end()),
          [](auto& group) { return group.enabled; }
      );
      if (parent_disabled) {
        FetchLine();
      }
      else {
        if (pp_token.streq("elif")) {
          if_group_stack.back().Elif(EatConstexpr() != 0);
          // newline eaten in EatConstexpr()
        }
        else {
          auto identifier = detail::get_identifier(CurrentStream());
          bool has_def = definitions.contains(identifier) || StandardDefinitions.contains(identifier);
          if (pp_token.streq("elifdef")) {
            if_group_stack.back().Elif(has_def);
          }
          else {
            cotyl::Assert(pp_token.streq("elifndef"));
            if_group_stack.back().Elif(!has_def);
          }
          EatNewline();
        }
      }
    }
  }
  else if (pp_token.streq("else")) {
    if (if_group_stack.empty()) {
      throw PreprocessorError("Unexpected else group");
    }
    // no need to check current enabled status, 
    // won't change if we are in a nested disabled group
    if_group_stack.back().Else();
    EatNewline();
  }
  else if (pp_token.streq("endif")) {
    if (if_group_stack.empty()) {
      throw PreprocessorError("Unexpected endif");
    }
    if_group_stack.pop_back();
    EatNewline();
  }
  else if (!enabled) {
    // non-control flow preprocessing directives
    // we ignore any of these if the current group is disabled
    FetchLine();
  }
  else if (pp_token.streq("define")) {
    SkipBlanks(false);
    auto name = detail::get_identifier(CurrentStream());

    if (!InternalIsEOS() && NextCharacter() == '(') {
      // functional macro
      cotyl::vector<cotyl::CString> arguments = {};
      bool variadic = false;
      CurrentStream().Skip();  // skip ( character

      SkipBlanks(false);
      // we do end of stream cannot happen here,
      // since the macro needs to be complete
      for (char c = NextCharacter(); c != ')'; c = NextCharacter()) {
        if (detail::is_valid_ident_start(c)) {
          auto arg = detail::get_identifier(CurrentStream());
          arguments.emplace_back(std::move(arg));
          SkipBlanks(false);

          if (NextCharacter() == ',') {
            // more arguments
            EatNextCharacter(',');
          }
        }
        else if (c == '.') {
          // variadic macro: #define variadic(arg1, arg2, ...)
          CurrentStream().EatSequence('.', '.', '.');
          SkipBlanks(false);
          variadic = true;
          // there cannot be any more arguments after this
          break;
        }
        else {
          throw cotyl::FormatExceptStr<PreprocessorError>("Unexpected character in macro argument list: %c", NextCharacter());
        }

        SkipBlanks(false);
      }
      EatNextCharacter(')');

      auto value = FetchLine();

      // only save definitions if current group is enabled
      if (enabled) {
        definitions.emplace(name, Definition(std::move(arguments), variadic, std::move(value)));
      }
    }
    else {
      auto value = FetchLine();

      // only save definitions if current group is enabled
      if (enabled) {
        definitions.emplace(name, Definition(std::move(value)));
      }
    }
    // newline eaten in FetchLine()
  }
  else if (pp_token.streq("undef")) {
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    definitions.erase(identifier);
    EatNewline();
  }
  else if (pp_token.streq("line")) {
    // correct for newline on this line
    CurrentLine() = EatConstexpr() - 1;
    // newline eaten in EatConstexpr() >> FetchLine()
  }
  else if (pp_token.streq("error")) {
    auto message = FetchLine();
    throw cotyl::FormatExcept<PreprocessorError>("error: %s", message.c_str());
  }
  else if (pp_token.streq("pragma")) {
    throw cotyl::UnimplementedException("#pragma preprocessing directive");
  }
  else if (pp_token.streq("include")) {
    Include();
  }
  else {
    // garbage
    FetchLine();
  }
  cotyl::Assert(is_newline, "Expect to be at newline after preprocessor directive");
}

}