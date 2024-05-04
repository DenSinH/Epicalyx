#pragma once

#include "file/File.h"
#include "file/SString.h"

#include "Tokenizer.h"
#include "parser/ExpressionParser.h"

#include "Default.h"
#include "Containers.h"
#include "CString.h"
#include "Exceptions.h"
#include "swl/variant.hpp"

#include <stack>
#include <queue>
#include <utility>


namespace epi {

struct PreprocessorError : cotyl::Exception {
  PreprocessorError(std::string&& message) : 
      Exception("Preprocessor Error", std::move(message)) { }
};


struct Preprocessor final : cotyl::Stream<char>, ExpressionParser {
  Preprocessor(const std::string& filename);

  void PrintLoc(std::ostream& out) const final;
  ast::pExpr ResolveIdentifier(cotyl::CString&& name) const;

protected:
  char GetNew() final;
  bool IsEOS() final;
  

private:
  mutable bool block_macro_expansion = false;
  mutable Tokenizer this_tokenizer;

  // check if we are at EOS, WITHOUT accounting for
  // the pre_processing_queue
  bool InternalIsEOS();

  struct Definition;
  using MacroMap = cotyl::unordered_map<cotyl::CString, Definition>;

  struct FileStream {
    FileStream(const std::string& name) : stream{name}, name{name} { }

    File stream;
    std::string name;
    u64 line = 1;  // line indexing starts at 1
  };

  struct Definition {
    // a segment is either a string or an argument index
    using segment_t = swl::variant<cotyl::CString, i32>;
    using value_t = cotyl::vector<segment_t>;
    struct Arguments {
      size_t count;
      bool variadic;
    };

    Definition(cotyl::vector<cotyl::CString>&& args, bool variadic, cotyl::CString&& val) :
        arguments{Arguments{args.size(), variadic}},
        value{Parse(std::move(args), variadic, std::move(val))} { }

    Definition(cotyl::CString&& value) : arguments{}, value{std::move(value)} { }

    static value_t Parse(cotyl::vector<cotyl::CString>&& args, bool variadic, cotyl::CString&& value);

    value_t value;
    std::optional<Arguments> arguments{};
  };

  struct MacroStream final : public cotyl::Stream<char> {
    MacroStream(cotyl::CString&& name, const Definition& def, cotyl::vector<cotyl::CString>&& arguments, cotyl::CString&& va_args) :
        name{std::move(name)}, 
        def{def}, 
        arguments{std::move(arguments)},
        va_args{std::move(va_args)},
        current_stream{InitialStream.view()},
        current_index{-1} {
        
    }
    MacroStream(cotyl::CString&& name, const Definition& def) : MacroStream{std::move(name), def, {}, cotyl::CString{""}} { }
    ~MacroStream() = default;

    void PrintLoc(std::ostream& out) const final;

    // check whether stream is currently expanding an
    // argument or a literal
    bool ExpandingArgument() const;

    cotyl::CString name;
  protected:
    char GetNew() final;
    bool IsEOS() final;
  
  private:
    const static cotyl::CString InitialStream;  // " "

    const Definition& def; // value
    cotyl::vector<cotyl::CString> arguments{};  // argument values
    cotyl::CString va_args{};

    bool eos = false;
    SString current_stream;
    int current_index;
  };

  struct IfGroup {
    enum class Type {
      If, Elif, Else
    };

    bool enabled;
    bool was_enabled;
    Type type;

    static IfGroup If(bool cond) {
      return IfGroup{
        .enabled = cond,
        .was_enabled = cond,
        .type = Type::If
      };
    }

    void Elif(bool cond) {
      if (type == Type::Else) {
        throw PreprocessorError("Unexpected elif group");
      }

      // only enable group if no earlier chain was taken and the condition is true
      enabled = !was_enabled && cond;
      was_enabled |= cond;
      type = Type::Elif;
    }

    void Else() {
      if (type == Type::Else) {
        throw PreprocessorError("Unexpected else group");
      }

      // only enable group if no earlier chain was taken
      enabled = !was_enabled;
      was_enabled = true;  // this is kind of redundant
      type = Type::Else;
    }
  };

  // contains queued up characters to be processed
  // used to handle strings, identifiers and strings of alphanumerical characters
  std::queue<char> pre_processor_queue{};

  // current expression being parsed for #if condition
  mutable std::optional<SString> expression{};

  // stack of macros and arguments to be processed before anything else
  mutable std::deque<MacroStream> macro_stack{};

  // stack of included files to be processed
  mutable std::deque<FileStream> file_stack{};
  bool is_newline = true;

  // stack of if groups that may or may not be enabled
  std::deque<IfGroup> if_group_stack{};

  // macro definitions, and a set of default definitions
  MacroMap definitions{};
  static const cotyl::unordered_map<cotyl::CString, cotyl::CString (Preprocessor::*)() const> StandardDefinitions;
  cotyl::CString FILE() const;
  cotyl::CString LINE() const;
  cotyl::CString DATE() const;
  cotyl::CString TIME() const;
  cotyl::CString STDC() const;
  cotyl::CString STDC_HOSTED() const;
  cotyl::CString STDC_VERSION() const;

  // a set of parsed files for tracking #pragma once directives
  cotyl::unordered_set<cotyl::CString> parsed_files{};

  // we want to reduce the "CurrentStream" usage as much as possible,
  // since it may introduce erroneous tracking of global preprocessor state
  // i.e. current newline status or line number
  // we comment on the use whenever we do use it, except in the wrapper
  // functions that handle the state updates properly (...NextCharacter)
  cotyl::Stream<char>& CurrentStream() const;
  void ClearEmptyStreams() const;
  u64& CurrentLine();
  std::string CurrentFile();

  bool Enabled() const;

  void SkipEscapedNewline();
  void SkipBlanks(bool skip_newlines = true);
  void SkipLineComment();
  void SkipMultilineComment();
  cotyl::CString FetchLine();
  char NextCharacter() const;
  char GetNextCharacter();
  void SkipNextCharacter();
  void EatNextCharacter(char c);
  // if it is known that no extra checks are needed, we use this faster version
  void SkipNextCharacterSimple();
  cotyl::CString GetNextProcessed();

  cotyl::CString GetMacroArgumentValue(bool variadic);
  static void ReplaceNewlines(cotyl::CString& value);
  void PushMacro(cotyl::CString&& name, const Definition& definition);
  void EatNewline();
  i64 EatConstexpr();
  void Include();
  std::string FindFile(const cotyl::CString& name, bool system);

  void PreprocessorDirective();
};

}