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
  for (const auto& file : file_stack) {
    std::cout << "in " << file.name << ":" << file.line << std::endl;
  }
  std::cout << std::endl;
  file_stack.back().stream.PrintLoc();
}

File& Preprocessor::CurrentStream() {
  return file_stack.back().stream;
}

bool Preprocessor::IsNewline() {
  return file_stack.back().is_newline;
}

void Preprocessor::SetNewline(bool status) {
  file_stack.back().is_newline = status;
}

char Preprocessor::NextCharacter() {
  if (CurrentStream().EOS()) {
    file_stack.pop_back();
    if (file_stack.empty()) {
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
        CurrentStream().SkipWhile([&](char c) {
          if (c == '\n') {
            file_stack.back().line++;
          }
          return std::isspace(c);
        });
      }
    }
    else if (c == '\n') {
      if (skip_newlines) {
        CurrentStream().Skip();
        SetNewline(true);
        file_stack.back().line++;
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

bool Preprocessor::IsEOS() {
  if (!pre_processor_queue.empty()) {
    return false;
  }
  return file_stack.size() == 1 && CurrentStream().EOS();
}

void Preprocessor::ExpectNewline() {
  SkipBlanks();
  if (!IsNewline()) {
    throw std::runtime_error("Expected newline");
  }
}

std::string Preprocessor::ExpectIdentifier() {
  std::stringstream stream{};
  if (CurrentStream().PredicateAfter(0, detail::is_valid_ident_start)) {
    while (CurrentStream().PredicateAfter(0, [](char k) { return std::isalnum(k) || k == '_'; })) {
      stream << CurrentStream().Get();
    }
  }
  const auto identifier = stream.str();
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

    if (std::isspace(c)) {
      SkipBlanks();
      return ' ';  // the exact whitespace character does not matter
    }

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
    else if (detail::is_valid_ident_start(c)) {
      SetNewline(false);
      // possible identifier, process entire identifier
      std::string identifier = ExpectIdentifier();

      // todo
      if (identifier == "__FILE__") {
        const auto& file = file_stack.back().name;
        pre_processor_queue.push('"');
        for (char k : file) {
          pre_processor_queue.push(k);
        }
        pre_processor_queue.push('"');
      }
      else if (identifier == "__LINE__") {
        const auto& line = std::to_string(file_stack.back().line);
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
  const std::string pp_token = ExpectIdentifier();

  if (pp_token == "if") {
    // todo
  }
  else if (pp_token == "ifdef") {
    SkipBlanks(false);
    auto identifier = ExpectIdentifier();
    // todo
  }
  else if (pp_token == "ifndef") {
    SkipBlanks(false);
    auto identifier = ExpectIdentifier();
    // todo
  }
  else if (pp_token == "define") {
    SkipBlanks(false);
    auto identifier = ExpectIdentifier();

    if (NextCharacter() == '(') {
      // functional macro
    }
    else {
      // normal macro
    }
    // todo
    ExpectNewline();
  }
  else if (pp_token == "undef") {
    SkipBlanks(false);
    auto identifier = ExpectIdentifier();

    definitions.erase(identifier);
    ExpectNewline();
  }
  else if (pp_token == "line") {
    // todo
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
  }
  else {
    throw cotyl::FormatExcept("Unexpected token after '#': %s", pp_token.c_str());
  }
}

}