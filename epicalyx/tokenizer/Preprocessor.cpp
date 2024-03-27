#include "Preprocessor.h"

#include "Tokenizer.h"
#include "Identifier.h"

#include "parser/ConstParser.h"

#include <ranges>


namespace epi {

void Preprocessor::PrintLoc() const {
  for (const auto& file : file_stack) {
    std::cout << "in " << file.name << ":" << file.line << std::endl;
  }
  std::cout << std::endl;
  file_stack.back().stream.PrintLoc();
}

cotyl::Stream<char>& Preprocessor::CurrentStream() {
  // macros go before the current expression, since expressions only
  // occur in #if statements, and the macro_stack is empty then
  if (!macro_stack.empty()) return macro_stack.back().stream;
  if (expression.has_value()) return expression.value();
  return file_stack.back().stream;
}

u64& Preprocessor::CurrentLine() {
  return file_stack.back().line;
}

std::string Preprocessor::CurrentFile() {
  // todo: full file path (escape backslashes)
  return file_stack.back().name;
}

bool Preprocessor::Enabled() const {
  // always enabled when parsing expressions
  if (expression.has_value()) {
    return true;
  }

  // only enabled if all nested groups are enabled
  return if_group_stack.empty() || std::all_of(
          if_group_stack.begin(),
          if_group_stack.end(),
          [](auto& group) { return group.enabled; }
  );
}

bool& Preprocessor::IsNewline() {
  return file_stack.back().is_newline;
}

char Preprocessor::NextCharacter() {
  while (CurrentStream().EOS()) {
    if (!macro_stack.empty()) {
      macro_stack.pop_back();
    }
    else {
      file_stack.pop_back();
      if (file_stack.empty()) {
        throw cotyl::EndOfFileException();
      }
      // insert newline character when file ends (for #include safety)
      return '\n';
    }
  }
  char c;
  if (!CurrentStream().Peek(c)) {
      throw cotyl::EndOfFileException();
  }
  return c;
}

void Preprocessor::SkipBlanks(bool skip_newlines) {
  // we do not want to crash on an end of stream while skipping whitespace
  while (!IsEOS()) {
    char c = NextCharacter();
    if (c == '\\') {
      // escaped newline (no newline, do increment line number)
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        CurrentStream().Skip();  // skip \ character
        CurrentStream().SkipWhile([&](char c) -> bool {
          if (c == '\n') {
            if (!macro_stack.empty()) {
              // newlines in macros do not count
              CurrentLine()++;
              return false;  // only skip one line with escaped newline
            }
          }
          return std::isspace(c);
        });
      }
    }
    else if (c == '\n') {
      if (skip_newlines || !macro_stack.empty()) {
        // skip the newline in both cases, but only update the line status
        // if we are not scanning a string
        CurrentStream().Skip();
        if (macro_stack.empty()) {
          IsNewline() = true;
          CurrentLine()++;
        }
      }
      else {
        break;
      }
    }
    else if (std::isspace(c)) {
      CurrentStream().Skip();
    }
    else {
      break;
    }
  }
}

void Preprocessor::SkipLineComment() {
  CurrentStream().EatSequence('/', '/');
  while (!IsEOS()) {
    char k = NextCharacter();
    if (k == '\\') {
      CurrentStream().Skip();

      // escaped newline
      SkipBlanks();
    }
    else if (k != '\n'){
      CurrentStream().Skip();
    }
    else {
      break;
    }
  }
}

void Preprocessor::SkipMultilineComment() {
  CurrentStream().EatSequence('/', '*');
  while (!CurrentStream().SequenceAfter(0, '*', '/')) {
    CurrentStream().Skip();
  }
  CurrentStream().Skip(2);  // skip */ characters
}

std::string Preprocessor::FetchLine() {
  cotyl::Assert(macro_stack.empty(), "Expected empty string stack in preprocessor line fetch");
  cotyl::Assert(!expression.has_value(), "Unexpected preprocessor line fetch while parsing expression");
  std::stringstream line;

  while (!IsEOS()) {
    char c = NextCharacter();
    if (c == '\\') {
      // escaped newline (no newline, do increment line number)
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        CurrentStream().Skip();  // skip \ character
        CurrentStream().SkipWhile([&](char c) -> bool {
          if (c == '\n') {
            // no need to check macro_stack.empty() since it is already asserted
            CurrentLine()++;
            return false;  // only skip one line with escaped newline
          }
          return std::isspace(c);
        });

        // add single whitespace character to line
        line << ' ';
      }
    }
    else if (c == '\n') {
      // end of line
      // do not eat the line ending
      // it is okay if allow_newline is false here because we do not eat the newline
      return line.str();
    }
    else if (CurrentStream().SequenceAfter(0, '/', '/')) {
      SkipLineComment();
      return line.str();
    }
    else if (CurrentStream().SequenceAfter(0, '/', '*')) {
      SkipMultilineComment();
    }
    else {
      CurrentStream().Skip();
      line << c;
    }
  }
  return line.str();
}

bool Preprocessor::IsEOS() {
  if (!pre_processor_queue.empty()) {
    // still processed characters to be fetched
    // may be filled with characeters from an expression that should be parsed
    return false;
  }
  if (expression.has_value()) {
    return expression.value().EOS();
  }
  if (!macro_stack.empty()) {
    // still string stack characters to be parsed
    return false;
  }
  return file_stack.size() == 1 && file_stack.back().stream.EOS();
}

void Preprocessor::EatNewline() {
  SkipBlanks();
  if (!IsNewline()) {
    throw std::runtime_error("Expected newline");
  }
}

i64 Preprocessor::EatConstexpr() {
  cotyl::Assert(macro_stack.empty() && !expression.has_value(), "Must start expression with empty macro stack and expression");
  cotyl::Assert(pre_processor_queue.empty(), "Must start preprocessor expression with empty pre_processor_queue");
  std::string line = FetchLine();

  expression = line;

  // the Tokenizer class does not allocate any memory anyway
  auto tokenizer = Tokenizer(*this);

  // ConstParser does not take up any memory (besides the vtable)
  // this saves us having to copy over the code for parsing
  // constant expressions
  auto parser = ConstParser(tokenizer);

  try {
    auto result = parser.EConstexpr();

    // we expect the bottom string (expression) to be fully parsed
    cotyl::Assert(macro_stack.empty(), "Found unexpanded macros after expression");
    expression = {};
    return result;
  }
  catch (cotyl::EndOfFileException& e) {
    throw std::runtime_error("Invalid expression");
  }
  catch (cotyl::UnexpectedIdentifierException& e) {
    throw std::runtime_error("Unexpected identifier in constant expression");
  }
}

std::string Preprocessor::GetNextProcessed(MacroExpansion macro_expansion) {
  // no need to check end of stream, since it is expected a new character even exists
  while (true) {
    char c = NextCharacter();

    // parse whitespace normally to count newlines properly when current group is not enabled
    if (std::isspace(c)) {
      SkipBlanks();
      return " ";  // the exact whitespace character does not matter
    }

    // potential new if group
    if (c == '#') {
      // preprocessing directives can only occur at the start of a line
      if (!IsNewline()) {
        throw cotyl::FormatExcept("Stray '#' in program");
      }
      PreprocessorDirective();

      // insert newline after preprocessing directive
      IsNewline() = true;
      return "\n";
    }

    if (!Enabled()) {
      // skip non-macros and non-whitespace if current group is not enabled
      CurrentStream().Skip();
      continue;
    }

    if (detail::is_valid_ident_start(c)) {
      IsNewline() = false;
      // possible identifier, process entire identifier
      const std::string identifier = detail::get_identifier(CurrentStream());

      if (identifier == "__FILE__") {
        const auto& file = CurrentFile();
        return cotyl::FormatStr("\"%s\"", file);
      }
      else if (identifier == "__LINE__") {
        const auto& line = std::to_string(CurrentLine());
        return cotyl::FormatStr("\"%s\"", line);
      }
      else {
        if (macro_expansion == MacroExpansion::Normal) {
          if (!macro_stack.empty() && macro_stack.back().arguments.contains(identifier)) {
            PushMacro(identifier, macro_stack.back().arguments.at(identifier));
          }
          else if (definitions.contains(identifier)) {
            PushMacro(identifier, definitions.at(identifier));
          }
          else {
            return identifier;
          }
        }
        else if (macro_expansion == MacroExpansion::Argument &&
            !macro_stack.empty() && macro_stack.back().arguments.contains(identifier)) {
          PushMacro(identifier, macro_stack.back().arguments.at(identifier));
        }
        else {
          // normal identifier
          return identifier;
        }
      }
    }
    else if (c == '/' && CurrentStream().SequenceAfter(0, '/', '/')) {
      // comment
      SkipLineComment();

      // eat line ending
      CurrentStream().Eat('\n');

      // insert newline after comment
      IsNewline() = true;
      return "\n";
    }
    else if (c == '/' && CurrentStream().SequenceAfter(0, '/', '*')) {
      /* multi-line comment */
      SkipMultilineComment();
    }
    else if (c == '"' || c == '\'') {
      // strings have to be fetched fully
      std::stringstream string;

      CurrentStream().Skip();
      const char quote = c;
      string << quote;

      while (NextCharacter() != quote) {
        if (NextCharacter() == '\\') {
          // feed 2 characters when escaped
          // note that we do feed the escape code to be parsed in the tokenizer
          string << '\\';
          CurrentStream().Skip();
          string << CurrentStream().Get();
        }
        else {
          string << CurrentStream().Get();
        }
      }

      // end string
      CurrentStream().Eat(quote);
      string << quote;
      return string.str();
    }
    else {
      // other character
      IsNewline() = false;

      if (detail::is_valid_ident_char(c)) {
        std::stringstream ident_char_string;
        // to prevent stuff like 1macro to parse as 1<macro definition>
        // we need to eat all valid identifier characters until we find one that is not
        while (CurrentStream().PredicateAfter(0, detail::is_valid_ident_char)) {
          ident_char_string << CurrentStream().Get();
        }
        return ident_char_string.str();
      }
      else {
        CurrentStream().Eat(c);
        return std::string{c};
      }
    }
  }
}

char Preprocessor::GetNew() {
  // no need to check end of stream, since it is expected a new character even exists
  while (true) {
    if (!pre_processor_queue.empty()) {
      char c = pre_processor_queue.front();
      pre_processor_queue.pop();
      return c;
    }

    for (const auto& c : GetNextProcessed(MacroExpansion::Normal)) {
      pre_processor_queue.push(c);
    }
  }
}

void Preprocessor::PreprocessorDirective() {
  CurrentStream().Eat('#');
  IsNewline() = false;

  SkipBlanks();
  if (IsNewline()) {
    // empty line after # character is allowed
    return;
  }

  // read identifier
  const std::string pp_token = detail::get_identifier(CurrentStream());

  if (pp_token == "if") {
    SkipBlanks(false);
    if_group_stack.push_back(IfGroup::If(EatConstexpr() != 0));
    EatNewline();
  }
  else if (pp_token == "ifdef") {
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    if_group_stack.push_back(IfGroup::If(definitions.contains(identifier)));

    EatNewline();
  }
  else if (pp_token == "ifndef") {
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    if_group_stack.push_back(IfGroup::If(!definitions.contains(identifier)));

    EatNewline();
  }
  else if (pp_token == "elif") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    if_group_stack.back().Elif(EatConstexpr() != 0);
    EatNewline();
  }
  else if (pp_token == "elifdef") {
    // non-standard
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    if_group_stack.back().Elif(definitions.contains(identifier));

    EatNewline();
  }
  else if (pp_token == "elifndef") {
    // non-standard
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    if_group_stack.back().Elif(!definitions.contains(identifier));

    EatNewline();
  }
  else if (pp_token == "else") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected else group");
    }
    if_group_stack.back().Else();

    EatNewline();
  }
  else if (pp_token == "endif") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected end if preprocessor if group");
    }
    if_group_stack.pop_back();
    EatNewline();
  }
  else if (pp_token == "define") {
    SkipBlanks(false);
    auto name = detail::get_identifier(CurrentStream());
    Definition def = {};

    if (!IsEOS() && NextCharacter() == '(') {
      // functional macro
      def.arguments = Definition::Arguments{};
      auto& arguments = def.arguments.value();
      CurrentStream().Skip();  // skip ( character

      SkipBlanks(false);
      // we do end of stream cannot happen here,
      // since the macro needs to be complete
      while (NextCharacter() != ')') {
        if (detail::is_valid_ident_start(NextCharacter())) {
          auto arg = detail::get_identifier(CurrentStream());
          arguments.list.push_back(arg);
          SkipBlanks(false);

          if (NextCharacter() == ',') {
            // more arguments
            CurrentStream().Eat(',');
          }
        }
        else if (NextCharacter() == '.') {
          // variadic macro: #define variadic(arg1, arg2, ...)
          CurrentStream().EatSequence('.', '.', '.');
          SkipBlanks(false);
          arguments.variadic = true;
          // there cannot be any more arguments after this
          break;
        }
        else {
          throw cotyl::FormatExceptStr("Unexpected character in macro argument list: %c", NextCharacter());
        }

        SkipBlanks(false);
      }
      CurrentStream().Eat(')');
    }

    // fetch macro definition
    def.value = FetchLine();
  
    // only save definitions if current group is enabled
    if (Enabled()) {
      definitions[name] = def;
    }
    
    EatNewline();
  }
  else if (pp_token == "undef") {
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());

    definitions.erase(identifier);
    EatNewline();
  }
  else if (pp_token == "line") {
    CurrentLine() = EatConstexpr();
    EatNewline();
  }
  else if (pp_token == "error") {
    std::stringstream message{};
    while (!CurrentStream().IsAfter(0, '\n')) {
      message << CurrentStream().Get();
    }
    throw cotyl::FormatExcept("error: %s", message.str().c_str());
  }
  else if (pp_token == "pragma") {
    // todo
    throw cotyl::UnimplementedException("#pragma preprocessing directive");
  }
  else if (pp_token == "include") {
    // todo
    throw cotyl::UnimplementedException("#include preprocessing directive");
  }
  else {
    throw cotyl::FormatExcept("Unexpected token after '#': %s", pp_token.c_str());
  }
}

std::string Preprocessor::GetMacroArgumentValue(bool variadic) {
  std::stringstream value;
  SkipBlanks();

  unsigned paren_count = 0;
  while (true) {
    // a comma or a brace is a part of a string if and only if the pre_processor_queue is not empty
    const char next = NextCharacter();
    if (next == ',' && !variadic && paren_count == 0) {
      // next argument
      return value.str();
    }
    else if (next == ')') {
      // if paren_count is 0 there are no more arguments and this argument is done
      if (paren_count == 0) return value.str();
      else {
        // otherwise, track the parenthesis
        paren_count--;
        CurrentStream().Eat(')');
        value << ')';
      }
    }
    else if (next == '(') {
      // track parentheses
      CurrentStream().Eat('(');
      paren_count++;
      value << '(';
    }
    else {
      // add next single-shot characters without expanding macros to macro definition
      for (const auto& n : GetNextProcessed(MacroExpansion::Argument)) {
        value << n;
      }
    }
  }
}

bool Preprocessor::IsDefinition(const std::string& name, Definition& definition) {
  if (!macro_stack.empty()) {
    // only top level macro argument names should be replaced
    if (macro_stack.back().arguments.contains(name)) {
      definition = macro_stack.back().arguments.at(name);
      return true;
    }
  }
  if (definitions.contains(name)) {
    definition = definitions.at(name);
    return true;
  }
  return false;
}

void Preprocessor::PushMacro(const std::string& name, const Definition& definition) {
  if (definition.value.empty()) {
    // no use pushing an empty macro
    return;
  }

  if (definition.arguments.has_value()) {
    const auto& arguments = definition.arguments.value();
    MacroMap arg_values{};
    // when parsing macro usage, newlines can be skipped just fine
    SkipBlanks();
    CurrentStream().Eat('(');
    for (const auto& arg : arguments.list) {
      arg_values[arg] = {GetMacroArgumentValue(false)};

      if (&arg != &arguments.list.back() || arguments.variadic) {
        // not last element
        CurrentStream().Eat(',');
      }
    }
    if (arguments.variadic && NextCharacter() != ')') {
      arg_values["__VA_ARGS__"] = {GetMacroArgumentValue(true)};
    }
    CurrentStream().Eat(')');

    macro_stack.emplace_back(name, definition.value, std::move(arg_values));
  }
  else {
    macro_stack.emplace_back(name, definition.value);
  }
}

}