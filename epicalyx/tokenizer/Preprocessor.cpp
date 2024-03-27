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

cotyl::Stream<char>& Preprocessor::CurrentStream() const {
  // macros go before the current expression, since expressions only
  // occur in #if statements, and the macro_stack is empty then
  if (!macro_stack.empty()) return macro_stack.back().stream;
  if (expression.has_value()) return expression.value();
  if (file_stack.empty()) {
    throw cotyl::EndOfFileException();
  }
  return file_stack.back().stream;
}

char Preprocessor::NextCharacter() const {
  while (CurrentStream().EOS()) {
    if (!macro_stack.empty()) {
      macro_stack.pop_back();
    }
    else if (expression.has_value()) {
      // parsing expression which is at the end, no next character
      throw cotyl::EndOfFileException();
    }
    else {
      // parsing from file stack, file is at end
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

char Preprocessor::GetNextCharacter() {
  char c;
  if (macro_stack.empty() && !expression.has_value()) {
    // parsing from file stack
    if (CurrentStream().EOS()) {
      // insert newline
      file_stack.pop_back();
      c = '\n';
    }
    else {
      c = CurrentStream().Get();
      if (c == '\n') {
        is_newline = true;
        CurrentLine()++;
      }
    }
  }
  else {
    c = CurrentStream().Get();
  }

  if (!std::isspace(c)) {
    is_newline = false;
  }
  return c;
}

void Preprocessor::SkipNextCharacter() {
  char c = GetNextCharacter();
  if (!std::isspace(c)) {
    is_newline = false;
  }
}

void Preprocessor::SkipNextCharacterSimple() {
  CurrentStream().Skip();
}

void Preprocessor::EatNextCharacter(char c) {
  char got = GetNextCharacter();
  if (c != got) {
    throw cotyl::FormatExcept("Unexpected character in preprocessor: got '%c', expected '%c'", got, c);
  }
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

void Preprocessor::SkipBlanks(bool skip_newlines) {
  // we do not want to crash on an end of stream while skipping whitespace
  while (!IsEOS()) {
    char c = NextCharacter();
    if (c == '\\') {
      // escaped newline (no newline, do increment line number)
      // we check the predicate after 1 character
      // we cannot do this using ...NextCharacter without asserting that
      // a next character even exists
      // the use of CurrentStream() is "fixed" here, since we can only encounter
      // newlines in file streams, in which case the line number is still updated
      // a \ at the end of a file is invalid regardless, in which case we will not
      // be skipping any blanks either
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        SkipNextCharacterSimple();  // skip \ character
        CurrentStream().SkipWhile([&](char c) -> bool {
          if (c == '\n') {
            // macros do not contain newline characters anyway
            CurrentLine()++;
            // in this case, the is_newline state does NOT change
            return false;  // only skip one line with escaped newline
          }
          return std::isspace(c);
        });
      }
    }
    else if (c == '\n') {
      if (skip_newlines || !macro_stack.empty()) {
        // skip the newline in both cases
        SkipNextCharacter();
      }
      else {
        break;
      }
    }
    else if (std::isspace(c)) {
      SkipNextCharacter();
    }
    else {
      break;
    }
  }
}

void Preprocessor::SkipLineComment() {
  EatNextCharacter('/');
  EatNextCharacter('/');
  is_newline = false;
  while (!IsEOS() && !is_newline) {
    SkipNextCharacter();
  }
}

void Preprocessor::SkipMultilineComment() {
  EatNextCharacter('/');
  EatNextCharacter('*');
  while (!CurrentStream().SequenceAfter(0, '*', '/')) {
    // we DO check for newline state updates here
    SkipNextCharacter();
  }
  // skip */ characters
  EatNextCharacter('*');
  EatNextCharacter('/');
  // always on newline after multiline comment
  is_newline = true;
}

std::string Preprocessor::FetchLine() {
  cotyl::Assert(macro_stack.empty(), "Expected empty string stack in preprocessor line fetch");
  cotyl::Assert(!expression.has_value(), "Unexpected preprocessor line fetch while parsing expression");
  std::stringstream line;

  while (!IsEOS()) {
    char c = NextCharacter();
    if (c == '\\') {
      // escaped newline (no newline, do increment line number)
      // see SkipBlanks comment about CurrentStream usage
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        SkipNextCharacterSimple();  // skip \ character
        CurrentStream().SkipWhile([&](char c) -> bool {
          if (c == '\n') {
            // by assertion, we are not scanning a macro
            // newlines have been stripped from macro definitions anyway
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
      // CurrentStream() usage okay, since we EatNextCharacter within the handler function
      // state should have been handled properly anyway
      SkipLineComment();
      return line.str();
    }
    else if (CurrentStream().SequenceAfter(0, '/', '*')) {
      // CurrentStream() usage okay, since we EatNextCharacter within the handler function
      // state should have been handled properly anyway
      SkipMultilineComment();
    }
    else {
      line << GetNextCharacter();
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
  if (!is_newline) {
    throw std::runtime_error("Expected newline");
  }
}

i64 Preprocessor::EatConstexpr() {
  cotyl::Assert(macro_stack.empty() && !expression.has_value(), "Must start expression with empty macro stack and expression");
  cotyl::Assert(pre_processor_queue.empty(), "Must start preprocessor expression with empty pre_processor_queue");
  std::string line = FetchLine();
  ReplaceNewlines(line);

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
      if (!is_newline) {
        throw cotyl::FormatExcept("Stray '#' in program");
      }
      PreprocessorDirective();

      // insert newline after preprocessing directive
      is_newline = true;
      return "\n";
    }

    if (!Enabled()) {
      // skip non-macros and non-whitespace if current group is not enabled
      SkipNextCharacter();
      continue;
    }

    if (detail::is_valid_ident_start(c)) {
      is_newline = false;
      // possible identifier, process entire identifier
      // we can always fetch identifiers from the current stream, as it does not cross any
      // state changing boundaries
      const std::string identifier = detail::get_identifier(CurrentStream());

      if (identifier == "__FILE__") {
        const auto& file = CurrentFile();
        return cotyl::FormatStr("\"%s\"", file);
      }
      else if (identifier == "__LINE__") {
        return std::to_string(CurrentLine());
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
      // CurrentStream usage is okay since we use EatNextCharacter in the handler function
      // state should have been handled okay for this to behave properly
      SkipLineComment();

      // eat line ending
      EatNextCharacter('\n');

      // insert newline after comment
      is_newline = true;
      return "\n";
    }
    else if (c == '/' && CurrentStream().SequenceAfter(0, '/', '*')) {
      /* multi-line comment */
      // CurrentStream usage is okay since we use EatNextCharacter in the handler function
      // state should have been handled okay for this to behave properly
      SkipMultilineComment();
    }
    else if (c == '"' || c == '\'') {
      // strings have to be fetched fully
      std::stringstream string;

      SkipNextCharacter();
      const char quote = c;
      string << quote;

      while (NextCharacter() != quote) {
        if (NextCharacter() == '\\') {
          // feed 2 characters when escaped
          // note that we do feed the escape code to be parsed in the tokenizer
          string << '\\';
          EatNextCharacter('\\');
          string << GetNextCharacter();  // todo: do not allow stream changes
        }
        else {
          string << GetNextCharacter();  // todo: do not allow stream changes
        }
      }

      // end string
      EatNextCharacter(quote);
      string << quote;
      return string.str();
    }
    else {
      // other character
      is_newline = false;

      if (detail::is_valid_ident_char(c)) {
        std::stringstream ident_char_string;
        // to prevent stuff like 1macro to parse as 1<macro definition>
        // we need to eat all valid identifier characters until we find one that is not
        // use of CurrentStream() is okay since we are not crossing any state changing boundaries
        while (CurrentStream().PredicateAfter(0, detail::is_valid_ident_char)) {
          ident_char_string << CurrentStream().Get();
        }
        return ident_char_string.str();
      }
      else {
        EatNextCharacter(c);
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
  EatNextCharacter('#');
  is_newline = false;

  SkipBlanks();
  if (is_newline) {
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
  }
  else if (pp_token == "ifndef") {
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    if_group_stack.push_back(IfGroup::If(!definitions.contains(identifier)));
  }
  else if (pp_token == "elif") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    if_group_stack.back().Elif(EatConstexpr() != 0);
  }
  else if (pp_token == "elifdef") {
    // non-standard
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    if_group_stack.back().Elif(definitions.contains(identifier));
  }
  else if (pp_token == "elifndef") {
    // non-standard
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    if_group_stack.back().Elif(!definitions.contains(identifier));
  }
  else if (pp_token == "else") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected else group");
    }
    if_group_stack.back().Else();
  }
  else if (pp_token == "endif") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected end if preprocessor if group");
    }
    if_group_stack.pop_back();
  }
  else if (pp_token == "define") {
    SkipBlanks(false);
    auto name = detail::get_identifier(CurrentStream());
    Definition def = {};

    if (!IsEOS() && NextCharacter() == '(') {
      // functional macro
      def.arguments = Definition::Arguments{};
      auto& arguments = def.arguments.value();
      SkipNextCharacterSimple();  // skip ( character

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
            EatNextCharacter(',');
          }
        }
        else if (NextCharacter() == '.') {
          // variadic macro: #define variadic(arg1, arg2, ...)
          EatNextCharacter('.');
          EatNextCharacter('.');
          EatNextCharacter('.');
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
      EatNextCharacter(')');
    }

    // fetch macro definition
    def.value = FetchLine();
    ReplaceNewlines(def.value);
  
    // only save definitions if current group is enabled
    if (Enabled()) {
      definitions[name] = def;
    }
  }
  else if (pp_token == "undef") {
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());

    definitions.erase(identifier);
  }
  else if (pp_token == "line") {
    CurrentLine() = EatConstexpr();
  }
  else if (pp_token == "error") {
    std::string message = FetchLine();
    throw cotyl::FormatExcept("error: %s", message.c_str());
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
  EatNewline();
}

void Preprocessor::ReplaceNewlines(std::string& value) {
  std::replace(value.begin(), value.end(), '\n', ' ');
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
    EatNextCharacter('(');
    for (const auto& arg : arguments.list) {
      auto arg_val = GetMacroArgumentValue(false);
      ReplaceNewlines(arg_val);
      arg_values[arg] = {arg_val};

      if (&arg != &arguments.list.back() || arguments.variadic) {
        // not last element
        EatNextCharacter(',');
      }
    }
    if (arguments.variadic && NextCharacter() != ')') {
      auto arg_val = GetMacroArgumentValue(true);
      ReplaceNewlines(arg_val);
      arg_values["__VA_ARGS__"] = {arg_val};
    }
   EatNextCharacter(')');

    // definition value has already been cleaned on #define
    macro_stack.emplace_back(name, definition.value, std::move(arg_values));
  }
  else {
    // definition value has already been cleaned on #define
    macro_stack.emplace_back(name, definition.value);
  }
}

}