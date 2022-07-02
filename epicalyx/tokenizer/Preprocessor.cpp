#include "Preprocessor.h"

#include "file/SString.h"
#include "Tokenizer.h"
#include "parser/ConstParser.h"


namespace epi {

namespace detail {

// todo: also use these for tokenizer
constexpr bool is_valid_ident_start(char c) {
  return std::isalpha(c) || c == '_';
}

constexpr bool is_valid_ident_char(char c) {
  return std::isalnum(c) || c == '_';
}

}

void Preprocessor::PrintLoc() const {
  if (include_stack.empty()) {
    in_stream.stream.PrintLoc();
    return;
  }

  for (const auto& file : include_stack) {
    std::cout << "in " << file.name << ":" << file.line << std::endl;
  }
  std::cout << std::endl;
  include_stack.back().stream.PrintLoc();
}

cotyl::Stream<char>& Preprocessor::CurrentStream() {
  if (include_stack.empty()) return in_stream.stream;
  return include_stack.back().stream;
}

int& Preprocessor::CurrentLine() {
  if (include_stack.empty()) return in_stream.line;
  return include_stack.back().line;
}

bool Preprocessor::Enabled() const {
  // only enabled if all nested groups are enabled
  return if_group_stack.empty() || std::all_of(
          if_group_stack.begin(),
          if_group_stack.end(),
          [](auto& group) { return group.enabled; }
  );
}

bool Preprocessor::IsNewline() const {
  if (include_stack.empty()) {
    return in_stream.is_newline;
  }
  else {
    return include_stack.back().is_newline;
  }
}

void Preprocessor::SetNewline(bool status) {
  if (include_stack.empty()) {
    in_stream.is_newline = status;
  }
  else {
    include_stack.back().is_newline = status;
  }
}

char Preprocessor::NextCharacter() {
  if (CurrentStream().EOS()) {
    if (include_stack.empty()) {
      throw std::runtime_error("Unexpected end of file");
    }
    include_stack.pop_back();

    // insert newline character when file ends (for #include safety)
    return '\n';
  }
  char c;
  if (!CurrentStream().Peek(c)) {
    throw std::runtime_error("Unexpected end of file");
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
            CurrentLine()++;
            return false;  // only skip one line with escaped newline
          }
          return std::isspace(c);
        });
      }
    }
    else if (c == '\n') {
      if (skip_newlines) {
        CurrentStream().Skip();
        SetNewline(true);
        CurrentLine()++;
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

std::string Preprocessor::FetchLine() {
  std::stringstream line;

  while (!IsEOS()) {
    char c = NextCharacter();
    if (c == '\\') {
      // escaped newline (no newline, do increment line number)
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        CurrentStream().Skip();  // skip \ character
        CurrentStream().SkipWhile([&](char c) -> bool {
          if (c == '\n') {
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
      return line.str();
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
    return false;
  }
  return include_stack.empty() && in_stream.stream.EOS();
}

void Preprocessor::EatNewline() {
  SkipBlanks();
  if (!IsNewline()) {
    throw std::runtime_error("Expected newline");
  }
}

std::string Preprocessor::EatIdentifier() {
  std::stringstream stream{};
  if (CurrentStream().PredicateAfter(0, detail::is_valid_ident_start)) {
    while (CurrentStream().PredicateAfter(0, [](char k) { return std::isalnum(k) || k == '_'; })) {
      stream << CurrentStream().Get();
    }
  }

  auto identifier = stream.str();
  if (identifier.empty()) {
    throw std::runtime_error("Expected identifier");
  }
  return identifier;
}

i64 Preprocessor::EatConstexpr() {
  std::string line = FetchLine();

  auto line_stream = SString(line);

  // copy over definitions to allow using preprocessing definitions in condition
  // todo: prevent having to allocate new preprocessor and copy over data
  auto preprocessor = Preprocessor(line_stream);
  preprocessor.definitions = definitions;
  preprocessor.in_stream.line = CurrentLine();
  preprocessor.in_stream.is_newline = false;

  // the Tokenizer class does not allocate any memory anyway
  auto tokenizer = Tokenizer(preprocessor);

  // ConstParser does not take up any memory (besides the vtable)
  // this saves us having to copy over the code for parsing
  // constant expressions
  auto parser = ConstParser(tokenizer);

  return parser.EConstexpr();
}

char Preprocessor::GetNew() {

  // no need to check end of stream, since it is expected a new character even exists
  while (true) {
    if (!pre_processor_queue.empty()) {
      char c = pre_processor_queue.front();
      pre_processor_queue.pop();
      return c;
    }

    char c = NextCharacter();

    // parse whitespace normally to count newlines properly when current group is not enabled
    if (std::isspace(c)) {
      SkipBlanks();
      return ' ';  // the exact whitespace character does not matter
    }

    // potential new if group
    if (c == '#') {
      // preprocessing directives can only occur at the start of a line
      if (!IsNewline()) {
        throw cotyl::FormatExcept("Stray '#' in program");
      }
      PreprocessorDirective();

      // insert newline after preprocessing directive
      SetNewline(true);
      return '\n';
    }

    if (!Enabled()) {
      // skip non-macros and non-whitespace if current group is not enabled
      CurrentStream().Skip();
      continue;
    }

    if (detail::is_valid_ident_start(c)) {
      SetNewline(false);
      // possible identifier, process entire identifier
      std::string identifier = EatIdentifier();

      // todo: other macros (and recursive processing)
      if (identifier == "__FILE__") {
        // todo: full file path (escape backslashes)
        const auto& file = include_stack.back().name;
        pre_processor_queue.push('"');
        for (char k : file) {
          pre_processor_queue.push(k);
        }
        pre_processor_queue.push('"');
      }
      else if (identifier == "__LINE__") {
        const auto& line = std::to_string(include_stack.back().line);
        pre_processor_queue.push('"');
        for (char k : line) {
          pre_processor_queue.push(k);
        }
        pre_processor_queue.push('"');

      }
      else {
        for (char k : identifier) {
          pre_processor_queue.push(k);
        }
      }
    }
    else if (c == '/' && CurrentStream().SequenceAfter(0, '/', '/')) {
      // comment
      while (!IsEOS()) {
        char k = NextCharacter();
        if (k == '\\') {
          CurrentStream().Skip(2);  // will also skip newlines
        }
        else if (k != '\n'){
          CurrentStream().Skip();
        }
        else {
          break;
        }
      }

      // insert newline after comment
      SetNewline(true);
      return '\n';
    }
    else if (c == '/' && CurrentStream().SequenceAfter(0, '/', '*')) {
      /* multi-line comment */
      CurrentStream().Skip(2);  // skip /* characters
      while (!CurrentStream().SequenceAfter(0, '*', '/')) {
        CurrentStream().Skip();
      }
      CurrentStream().Skip(2);  // skip */ characters

      // insert newline after comment
      SetNewline(true);
      return '\n';
    }
    else {
      // other character
      SetNewline(false);
      CurrentStream().Eat(c);
      return c;
    }
  }
}

void Preprocessor::PreprocessorDirective() {
  CurrentStream().Eat('#');
  SetNewline(false);

  SkipBlanks();
  if (IsNewline()) {
    // empty line after # character is allowed
    return;
  }

  // read identifier
  const std::string pp_token = EatIdentifier();

  if (pp_token == "if") {
    SkipBlanks(false);
    if_group_stack.push_back(IfGroup::If(EatConstexpr() != 0));
    EatNewline();
  }
  else if (pp_token == "ifdef") {
    SkipBlanks(false);
    auto identifier = EatIdentifier();
    if_group_stack.push_back(IfGroup::If(definitions.contains(identifier)));

    EatNewline();
  }
  else if (pp_token == "ifndef") {
    SkipBlanks(false);
    auto identifier = EatIdentifier();
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
    auto identifier = EatIdentifier();
    if_group_stack.back().Elif(definitions.contains(identifier));

    EatNewline();
  }
  else if (pp_token == "elifndef") {
    // non-standard
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    auto identifier = EatIdentifier();
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
    auto name = EatIdentifier();
    Definition def = {};

    if (!IsEOS() && NextCharacter() == '(') {
      // functional macro
      def.arguments = {};
      auto& arguments = def.arguments.value();
      CurrentStream().Skip();  // skip ( character

      SkipBlanks(false);
      // we do end of stream cannot happen here,
      // since the macro needs to be complete
      if (NextCharacter() != ')') {
        while (true) {
          auto arg = EatIdentifier();
          arguments.list.push_back(arg);
          SkipBlanks(false);

          if (NextCharacter() == ')') {
            // end of argument list
            CurrentStream().Eat(')');
            break;
          }
          else {
            // more arguments
            CurrentStream().Eat(',');
          }

          if (NextCharacter() == '.') {
            // variadic macro: #define variadic(arg1, arg2, ...)
            CurrentStream().EatSequence('.', '.', '.');
            SkipBlanks(false);
            CurrentStream().Eat(')');
            arguments.variadic = true;
            break;
          }

          SkipBlanks(false);
        }
      }
      else {
        CurrentStream().Eat(')');
      }
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
    auto identifier = EatIdentifier();

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
    throw std::runtime_error("Unimplemented: #pragma preprocessing directive");
  }
  else {
    throw cotyl::FormatExcept("Unexpected token after '#': %s", pp_token.c_str());
  }
}

}