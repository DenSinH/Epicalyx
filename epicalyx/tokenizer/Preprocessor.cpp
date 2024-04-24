#include "Preprocessor.h"

#include "Tokenizer.h"
#include "Identifier.h"

#include "parser/ConstParser.h"

#include "SStream.h"
#include "CustomAssert.h"
#include "Decltype.h"

#include <ranges>
#include <iostream>


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
  if (!macro_stack.empty()) return macro_stack.back();
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

void Preprocessor::SkipEscapedNewline() {
  // escaped newline (no newline, do increment line number)
  // we check the predicate after 1 character
  // we cannot do this using ...NextCharacter without asserting that
  // a next character even exists
  // the use of CurrentStream() is "fixed" here, since we can only encounter
  // newlines in file streams, in which case the line number is still updated
  // a \ at the end of a file is invalid regardless, in which case we will not
  // be skipping any blanks either
  EatNextCharacter('\\');
  cotyl::Assert(CurrentStream().PredicateAfter(0, std::isspace));

  bool first_newline = true;
  CurrentStream().SkipWhile([&](char c) -> bool {
    if (c == '\n') {
      if (first_newline) {
        first_newline = false;  // only skip one line with escaped newline
        // by assertion, we are not scanning a macro
        // newlines have been stripped from macro definitions anyway
        CurrentLine()++;
        return true;
      }
      return false;  // don't skip multiple newlines
    }
    return std::isspace(c);
  });
}

void Preprocessor::SkipBlanks(bool skip_newlines) {
  // we do not want to crash on an end of stream while skipping whitespace
  while (!IsEOS()) {
    char c = NextCharacter();
    if (c == '\\') {
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        SkipEscapedNewline();
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
    if (NextCharacter() =='\\' && CurrentStream().PredicateAfter(1, std::isspace)) {
      SkipEscapedNewline();
    }
    else {
      SkipNextCharacter();
    }
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
  cotyl::StringStream line{};

  while (!IsEOS()) {
    char c = NextCharacter();
    if (c == '\\') {

      // possible escaped newline (no newline, do increment line number)
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        SkipEscapedNewline();

        // add single whitespace character to line
        line << ' ';
      }
      else {
        SkipNextCharacterSimple();  // skip \ character
        line << '\\';
      }
    }
    else if (c == '\n') {
      // end of line
      EatNextCharacter('\n');

      // it is okay if allow_newline is false here because we do not eat the newline
      return line.finalize();
    }
    else if (CurrentStream().SequenceAfter(0, '/', '/')) {
      // CurrentStream() usage okay, since we EatNextCharacter within the handler function
      // state should have been handled properly anyway
      SkipLineComment();
      return line.finalize();
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
  return line.finalize();
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

  expression = &line;

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
    expression.reset();

    // we fetched the whole line, so we are always at a new line after this
    is_newline = true;
    return result;
  }
  catch (cotyl::EndOfFileException& e) {
    throw std::runtime_error("Invalid expression");
  }
  catch (cotyl::UnexpectedIdentifierException& e) {
    throw std::runtime_error("Unexpected identifier in constant expression");
  }
}

std::string Preprocessor::GetNextProcessed() {
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
      else if (definitions.contains(identifier)) {
        PushMacro(identifier, definitions.at(identifier));
      }
      else {
        return identifier;
      }
    }
    else if (c == '/' && CurrentStream().SequenceAfter(0, '/', '/')) {
      // comment
      // CurrentStream usage is okay since we use EatNextCharacter in the handler function
      // state should have been handled okay for this to behave properly
      SkipLineComment();

      // expect newline after comment
      cotyl::Assert(is_newline);
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
      cotyl::StringStream string{};

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
      return string.finalize();
    }
    else {
      // other character
      is_newline = false;

      if (detail::is_valid_ident_char(c)) {
        cotyl::StringStream ident_char_string{};
        // to prevent stuff like 1macro to parse as 1<macro definition>
        // we need to eat all valid identifier characters until we find one that is not
        // use of CurrentStream() is okay since we are not crossing any state changing boundaries
        while (CurrentStream().PredicateAfter(0, detail::is_valid_ident_char)) {
          ident_char_string << CurrentStream().Get();
        }
        return ident_char_string.finalize();
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

    for (const auto& c : GetNextProcessed()) {
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
    if_group_stack.emplace_back(IfGroup::If(EatConstexpr() != 0));
    // newline eaten in EatConstexpr()
  }
  else if (pp_token == "ifdef") {
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    if_group_stack.emplace_back(IfGroup::If(definitions.contains(identifier)));
    EatNewline();
  }
  else if (pp_token == "ifndef") {
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());
    if_group_stack.emplace_back(IfGroup::If(!definitions.contains(identifier)));
    EatNewline();
  }
  else if (pp_token == "elif") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    if_group_stack.back().Elif(EatConstexpr() != 0);
    // newline eaten in EatConstexpr()
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

    if (!IsEOS() && NextCharacter() == '(') {
      // functional macro
      cotyl::vector<std::string> arguments = {};
      bool variadic = false;
      SkipNextCharacterSimple();  // skip ( character

      SkipBlanks(false);
      // we do end of stream cannot happen here,
      // since the macro needs to be complete
      while (NextCharacter() != ')') {
        if (detail::is_valid_ident_start(NextCharacter())) {
          auto arg = detail::get_identifier(CurrentStream());
          arguments.emplace_back(std::move(arg));
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
          variadic = true;
          // there cannot be any more arguments after this
          break;
        }
        else {
          throw cotyl::FormatExceptStr("Unexpected character in macro argument list: %c", NextCharacter());
        }

        SkipBlanks(false);
      }
      EatNextCharacter(')');

      std::string value = FetchLine() + " ";  // end macros in whitespace as to not glue preprocessing tokens
      ReplaceNewlines(value);

      // only save definitions if current group is enabled
      if (Enabled()) {
        definitions.emplace(name, Definition(arguments, variadic, value));
      }
    }
    else {
      std::string value = FetchLine() + " ";  // end macros in whitespace as to not glue preprocessing tokens
      ReplaceNewlines(value);

      // only save definitions if current group is enabled
      if (Enabled()) {
        definitions.emplace(name, Definition(std::move(value)));
      }
    }
    // newline eaten in FetchLine()
  }
  else if (pp_token == "undef") {
    SkipBlanks(false);
    auto identifier = detail::get_identifier(CurrentStream());

    definitions.erase(identifier);
    EatNewline();
  }
  else if (pp_token == "line") {
    CurrentLine() = EatConstexpr();
    // newline eaten in EatConstexpr()
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
  cotyl::Assert(is_newline, "Expect to be at newline after preprocessor directive");
}

void Preprocessor::ReplaceNewlines(std::string& value) {
  std::replace(value.begin(), value.end(), '\n', ' ');
}

Preprocessor::Definition::value_t Preprocessor::Definition::Parse(
  const cotyl::vector<std::string>& args,
  bool variadic,
  const std::string& value
) {
  value_t result{};
  auto valstream = SString(&value);
  cotyl::StringStream current_val{};
  char c;

  // any time we encounter a macro argument, we KNOW that current_val will be non-empty
  // so we can append a new string to the result
  while (valstream.Peek(c, 0)) {
    if (detail::is_valid_ident_start(c)) {
      auto ident = detail::get_identifier(valstream);
      if (variadic && ident == "__VA_ARGS__") {
        // variadic argument, emplace current intermediate text and reset
        result.emplace_back(current_val.finalize());
        current_val.clear();
        result.emplace_back(-1);
      }
      else {
        auto arg_idx = std::find(args.begin(), args.end(), ident);
        if (arg_idx != args.end()) {
          // argument id, emplace current intermediate text and reset
          result.emplace_back(current_val.finalize());
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
    result.emplace_back(current_val.finalize());
  }
  return result;
}

const std::string Preprocessor::MacroStream::InitialStream = " ";

char Preprocessor::MacroStream::GetNew() {
  if (eos) {
    throw cotyl::EndOfFileException();
  }
  else if (current_stream.EOS()) {
    if (++current_index < def.value.size()) {
      std::visit([&](const auto& seg) {
          using T = decltype_t(seg);
          if constexpr (std::is_same_v<T, i32>) {
            if (seg == -1) {
              current_stream = SString(&va_args);
            }
            else {
              current_stream = SString(&arguments[seg]);
            }
          }
          else if constexpr (std::is_same_v<T, std::string>) {
            current_stream = SString(&seg);
          }
      }, def.value[current_index]);
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

void Preprocessor::MacroStream::PrintLoc() const {

}

std::string Preprocessor::GetMacroArgumentValue(bool variadic) {
  cotyl::StringStream value{};
  SkipBlanks();

  unsigned paren_count = 0;
  while (true) {
    // a comma or a brace is a part of a string if and only if the pre_processor_queue is not empty
    const char next = NextCharacter();
    if (next == ',' && !variadic && paren_count == 0) {
      // next argument
      return value.finalize();
    }
    else if (next == ')') {
      // if paren_count is 0 there are no more arguments and this argument is done
      if (paren_count == 0) return value.finalize();
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
      for (const auto& n : GetNextProcessed()) {
        value << n;
      }
    }
  }
}

void Preprocessor::PushMacro(const std::string& name, const Definition& definition) {
  if (definition.value.empty()) {
    // no use pushing an empty macro
    return;
  }

  if (definition.arguments.has_value()) {
    const auto& arguments = definition.arguments.value();
    cotyl::vector<std::string> arg_values{};
    std::string va_args{};

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
    macro_stack.emplace_back(name, definition, std::move(arg_values), std::move(va_args));
  }
  else {
    // definition value has already been cleaned on #define
    macro_stack.emplace_back(name, definition);
  }
}

}