#include "Preprocessor.h"


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
  for (const auto& file : include_stack) {
    std::cout << "in " << file.name << ":" << file.line << std::endl;
  }
  std::cout << std::endl;
  include_stack.back().stream.PrintLoc();
}

File& Preprocessor::CurrentStream() {
  return include_stack.back().stream;
}

bool Preprocessor::Enabled() const {
  return if_group_stack.empty() || if_group_stack.top().enabled;
}

bool Preprocessor::IsNewline() const {
  return include_stack.back().is_newline;
}

void Preprocessor::SetNewline(bool status) {
  include_stack.back().is_newline = status;
}

char Preprocessor::NextCharacter() {
  if (CurrentStream().EOS()) {
    include_stack.pop_back();
    if (include_stack.empty()) {
      throw std::runtime_error("Unexpected end of file");
    }

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
  do {
    char c = NextCharacter();
    if (c == '\\') {
      // escaped newline (no newline, do increment line number)
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        CurrentStream().Skip();  // skip \ character
        CurrentStream().SkipWhile([&](char c) -> bool {
          if (c == '\n') {
            include_stack.back().line++;
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
        include_stack.back().line++;
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
  } while (true);
}

std::string Preprocessor::FetchLine() {
  std::stringstream line;

  do {
    char c = NextCharacter();
    if (c == '\\') {
      // escaped newline (no newline, do increment line number)
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        CurrentStream().Skip();  // skip \ character
        CurrentStream().SkipWhile([&](char c) -> bool {
          if (c == '\n') {
            include_stack.back().line++;
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
  } while (true);
}

bool Preprocessor::IsEOS() {
  if (!pre_processor_queue.empty()) {
    return false;
  }
  return include_stack.size() == 1 && CurrentStream().EOS();
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

char Preprocessor::GetNew() {

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
      while (true) {
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
    // todo
  }
  else if (pp_token == "ifdef") {
    SkipBlanks(false);
    auto identifier = EatIdentifier();
    if_group_stack.push(IfGroup::If(definitions.contains(identifier)));

    EatNewline();
  }
  else if (pp_token == "ifndef") {
    SkipBlanks(false);
    auto identifier = EatIdentifier();
    if_group_stack.push(IfGroup::If(!definitions.contains(identifier)));

    EatNewline();
  }
  else if (pp_token == "elif") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    // todo
    throw std::runtime_error("unimplemented: #elif");
  }
  else if (pp_token == "elifdef") {
    // non-standard
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    auto identifier = EatIdentifier();
    if_group_stack.top().Elif(definitions.contains(identifier));

    EatNewline();
  }
  else if (pp_token == "elifndef") {
    // non-standard
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected elif group");
    }
    SkipBlanks(false);
    auto identifier = EatIdentifier();
    if_group_stack.top().Elif(!definitions.contains(identifier));

    EatNewline();
  }
  else if (pp_token == "else") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected else group");
    }
    if_group_stack.top().Else();

    EatNewline();
  }
  else if (pp_token == "endif") {
    if (if_group_stack.empty()) {
      throw std::runtime_error("Unexpected end if preprocessor if group");
    }
    if_group_stack.pop();
    EatNewline();
  }
  else if (pp_token == "define") {
    SkipBlanks(false);
    auto name = EatIdentifier();
    Definition def = {};

    if (NextCharacter() == '(') {
      // functional macro
      def.arguments = {};
      auto& arguments = def.arguments.value();
      CurrentStream().Skip();  // skip ( character

      SkipBlanks(false);
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
    // todo (local parser for expression)
    throw std::runtime_error("Unimplemented: #line preprocessing directive");
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