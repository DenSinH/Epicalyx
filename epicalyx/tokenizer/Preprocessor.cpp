#include "Preprocessor.h"

#include "Identifier.h"
#include "SStream.h"
#include "CustomAssert.h"


namespace epi {

Preprocessor::Preprocessor(const std::string& in_stream) :
    this_tokenizer{*this},
    ExpressionParser{this_tokenizer} {
  file_stack.emplace_back(in_stream);
}

void Preprocessor::PrintLoc(std::ostream& out) const {
  for (const auto& file : file_stack) {
    out << "in " << file.name << ":" << file.line << std::endl;
  }
  out << std::endl;
  file_stack.back().stream.PrintLoc(out);
}

cotyl::Stream<char>& Preprocessor::CurrentStream() const {
  // macros go before the current expression, since expressions only
  // occur in #if statements, and the macro_stack is empty then
  ClearEmptyStreams();
  if (!state.macro_stack.empty()) return state.macro_stack.back();
  if (state.expression.has_value()) return state.expression.value();
  if (file_stack.empty()) throw cotyl::EOSError();
  return file_stack.back().stream;
}

char Preprocessor::NextCharacter() const {
  while (CurrentStream().EOS()) {
    if (!state.macro_stack.empty()) {
      state.macro_stack.pop_back();
    }
    else if (state.expression.has_value()) {
      // parsing expression which is at the end, no next character
      throw cotyl::EOSError();
    }
    else {
      // parsing from file stack, file is at end
      // insert newline character when file ends (for #include safety)
      return '\n';
    }
  }
  char c;
  if (!CurrentStream().Peek(c)) {
    throw cotyl::EOSError();
  }
  return c;
}

char Preprocessor::GetNextCharacter() {
  char c;
  if (state.macro_stack.empty() && !state.expression.has_value()) {
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
  // newline state is updated in GetNextCharacter
  char c = GetNextCharacter();
}

void Preprocessor::EatNextCharacter(char c) {
  char got = GetNextCharacter();
  if (c != got) {
    throw cotyl::FormatExcept<PreprocessorError>("Unexpected character in preprocessor: got '%c', expected '%c'", got, c);
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
  if (state.expression.has_value()) {
    return true;
  }

  // only enabled if all nested groups are enabled
  return std::all_of(
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
  while (!InternalIsEOS()) {
    char c = NextCharacter();
    if (c == '\\') {
      if (CurrentStream().PredicateAfter(1, std::isspace)) {
        SkipEscapedNewline();
      }
    }
    else if (c == '\n') {
      // empty macro streams cleared in InternalIsEOS
      if (skip_newlines || !state.macro_stack.empty()) {
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
  // newline status updated in EatNextCharacter
  EatNextCharacter('/');
  EatNextCharacter('/');
  
  while (!InternalIsEOS() && !is_newline) {
    if (NextCharacter() =='\\' && CurrentStream().PredicateAfter(1, std::isspace)) {
      SkipEscapedNewline();
    }
    else {
      SkipNextCharacter();
    }
  }
}

void Preprocessor::SkipMultilineComment() {
  bool old_is_newline = is_newline;
  CurrentStream().EatSequence('/', '*');
  while (!CurrentStream().SequenceAfter(0, '*', '/')) {
    // we do it this way to track the __LINE__ macro properly
    SkipNextCharacter();
  }
  // skip */ characters
  CurrentStream().EatSequence('*', '/');

  // multiline comment does not affect newline status
  is_newline = old_is_newline;
}

cotyl::CString Preprocessor::FetchLine() {
  cotyl::Assert((ClearEmptyStreams(), state.macro_stack.empty()), "Expected empty string stack in preprocessor line fetch");
  cotyl::Assert(!state.expression.has_value(), "Unexpected preprocessor line fetch while parsing expression");
  cotyl::StringStream line{};

  is_newline = false;
  while (!InternalIsEOS() && !is_newline) {
    line << GetNextChunk(false);
  }
  return line.cfinalize();
}

bool Preprocessor::IsEOS() {
  if (!state.pre_processor_queue.empty()) {
    // still processed characters to be fetched
    // may be filled with characeters from an expression that should be parsed
    return false;
  }
  return InternalIsEOS();
}

void Preprocessor::ClearEmptyStreams() const {
  while (!state.macro_stack.empty() && state.macro_stack.back().EOS()) {
    state.macro_stack.pop_back();
  }
  while (!file_stack.empty() && file_stack.back().stream.EOS()) {
    file_stack.pop_back();
  }
}

bool Preprocessor::InternalIsEOS() {
  ClearEmptyStreams();
  if (!state.macro_stack.empty()) {
    // still string stack characters to be parsed
    return false;
  }
  if (state.expression.has_value()) {
    // ignore file stack if we are parsing an expression
    return state.expression.value().EOS();
  }

  // file streams will have been removed in ClearEmptyStreams()
  return file_stack.empty();
}

void Preprocessor::EatNewline() {
  SkipBlanks();
  if (!is_newline) {
    throw PreprocessorError("Expected newline");
  }
}

cotyl::CString Preprocessor::GetNextChunk(bool do_preprocessing) {
  // no need to check end of stream, since it is expected a new character even exists
  while (true) {
    char c = NextCharacter();

    // parse whitespace normally to count newlines properly when current group is not enabled
    if (std::isspace(c)) {
      SkipBlanks();
      return cotyl::CString{' '};  // the exact whitespace character does not matter
    }

    // potential new if group
    if (c == '#') {
      if (!do_preprocessing) {
        return cotyl::CString{GetNextCharacter()};
      }
      // preprocessing directives can only occur at the start of a line
      if (!is_newline) {
        throw cotyl::FormatExcept<PreprocessorError>("Stray '#' in program");
      }
      PreprocessorDirective();

      // newline state is asserted after the preprocessing directive
      return cotyl::CString{'\n'};
    }

    if (do_preprocessing && !Enabled()) {
      // skip non-macros and non-whitespace if current group is not enabled
      SkipNextCharacter();
      continue;
    }

    if (detail::is_valid_ident_start(c)) {
      is_newline = false;
      // identifier, process entire identifier
      // we can always fetch identifiers from the current stream, as it does not cross any
      // state changing boundaries
      cotyl::CString identifier = detail::get_identifier(CurrentStream());
      
      // we may block macro expansion if we are checking a 
      // #if defined statement, or fetching macro arguments
      if (!do_preprocessing || block_macro_expansion) {
        return std::move(identifier);
      }

      if (StandardDefinitions.contains(identifier)) {
        return (this->*StandardDefinitions.at(identifier))();
      }
      else if (definitions.contains(identifier)) {  
        ClearEmptyStreams();
        if (!state.macro_stack.empty()) {
          const auto& current_macro = state.macro_stack.back();

          // arguments are already expanded where needed,
          // so we check whether the identifier should be used recursively
          if (current_macro.name == identifier) {
            // cannot recursively expand macro identifiers in macro definition
            // see 6.10.3.4 Rescanning and further replacement > 2
            // in ISO/IEC 9899:1999
            return identifier;
          }
        }
        const auto& def = definitions.at(identifier);
        if (def.arguments.has_value()) {
          SkipBlanks();
          if (!CurrentStream().IsAfter(0, '(')) {
            // callable macro that was not called
            return std::move(identifier);
          }
        }
        PushMacro(std::move(identifier), def);
        return GetNextChunk(do_preprocessing);
      }
      else {
        return std::move(identifier);
      }
    }
    else if (c == '/' && CurrentStream().SequenceAfter(0, '/', '/')) {
      // comment
      // CurrentStream usage is okay since we use EatNextCharacter in the handler function
      // state should have been handled okay for this to behave properly
      SkipLineComment();

      // expect newline after comment
      cotyl::Assert(is_newline);
      return cotyl::CString{'\n'};
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
      return string.cfinalize();
    }
    else {
      // other character

      if (detail::is_valid_ident_char(c)) {
        // newline state is not updated anywhere else
        is_newline = false;
        cotyl::StringStream ident_char_string{};
        // to prevent stuff like 1macro to parse as 1<macro definition>
        // we need to eat all valid identifier characters until we find one that is not
        // use of CurrentStream() is okay since we are not crossing any state changing boundaries
        while (CurrentStream().PredicateAfter(0, detail::is_valid_ident_char)) {
          ident_char_string << CurrentStream().Get();
        }
        return ident_char_string.cfinalize();
      }
      else {
        EatNextCharacter(c);
        return cotyl::CString{c};
      }
    }
  }
}

char Preprocessor::GetNew() {
  // no need to check end of stream, since it is expected a new character even exists
  while (true) {
    if (!state.pre_processor_queue.empty()) {
      char c = state.pre_processor_queue.front();
      state.pre_processor_queue.pop();
      return c;
    }

    for (const auto& c : GetNextChunk()) {
      state.pre_processor_queue.push(c);
    }
  }
}

}