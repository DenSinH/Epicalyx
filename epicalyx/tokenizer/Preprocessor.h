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

/*
 * We inherit from ExpressionParser, providing a custom
 * ResolveIdentifier method in order to properly parse
 * #if defined ...
 * The parser has a token stream input, which is a 
 * tokenizer that references *this as it's own char stream.
 * 
 * This greatly simplifies constant expression and string parsing.
 * */

struct Preprocessor final : cotyl::Stream<char>, ExpressionParser {
  Preprocessor(const std::string& filename);

  void PrintLoc(std::ostream& out) const final;

protected:
  // overridable inherited methods
  char GetNew() final;
  bool IsEOS() final;

  ast::pExpr ResolveIdentifier(cotyl::CString&& name) const;

private:
  // check if we are at EOS, WITHOUT accounting for
  // the pre_processing_queue
  bool InternalIsEOS() const;
  
  // we will block macro expansion when parsing 
  // constant expressions, in order to prevent
  // a macro from being expanded
  mutable bool block_macro_expansion = false;
  mutable Tokenizer this_tokenizer;
  

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
   * INPUT HANDLING
   * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  // Skip whitespace and first newline after \ character
  void SkipEscapedNewline();

  // skip whitespace, and optionally skip newlines as well
  void SkipBlanks(bool skip_newlines = true);

  // skip comments
  void SkipLineComment();
  void SkipMultilineComment();

  // fetch the rest of the current line
  // removes any comments, but does not expand macros
  // or handle preprocessing directives
  cotyl::CString FetchLine();

  // (force) peek the next character
  char NextCharacter() const;
  
  // get / skip / eat the next character
  char GetNextCharacter();
  void SkipNextCharacter();
  void EatNextCharacter(char c);

  // eat an (actual) newline from the currently read file
  void EatNewline();

  // get the next chunk of possibly parsed string data
  // this returns one of the following:
  // - a single whitespace character
  // - a quoted string (with either " or ')
  // - an valid identifier
  // - a sequence of identifier characters that may not begin
  //   with a valid identifier start (such as 1L)
  // - a single character
  cotyl::CString GetNextChunk(bool do_preprocessing = true);


  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
   * MACRO HANDLING
   * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  // macro definition struct
  // the value is built up of either:
  // literal strings, argument values and stringified argument values
  struct Definition {
    // a segment is either a string or an argument index
    struct Argument { 
      i32 arg_index;
      bool expand = true;
      bool concat_next = false; 
    };
    struct Hash { i32 arg_index; };

    using segment_t = swl::variant<cotyl::CString, Argument, Hash>;
    using value_t = cotyl::vector<segment_t>;
    struct Arguments {
      size_t count;
      bool variadic;
    };

    Definition(cotyl::vector<cotyl::CString>&& args, bool variadic, cotyl::CString&& val) :
        arguments{Arguments{args.size(), variadic}},
        value{Parse(std::move(args), variadic, std::move(val))} { }

    // parse value as non-variadic functional macro with no arguments
    Definition(cotyl::CString&& value) : 
        arguments{}, 
        value{Parse({}, false, std::move(value))} { }

    // parse definition from argument names, variadic status and raw string definition
    static value_t Parse(cotyl::vector<cotyl::CString>&& args, bool variadic, cotyl::CString&& value);

    value_t value;

    // no value means not callable
    std::optional<Arguments> arguments{};
  };

  // expanded macro stream
  // include name to prevent recursive macro expansion:
  // #define f(a) f(a)
  // expands f(1) to f(1)
  struct MacroStream final : public SString<cotyl::CString> {
    MacroStream(cotyl::CString&& name, cotyl::CString&& expanded) :
        SString{std::move(expanded)},
        name{std::move(name)} {
        
    }

    // override PrintLoc to display current macro name
    void PrintLoc(std::ostream& out) const final;

    cotyl::CString name;
  };
  
  // macro definitions, and a set of default definitions
  cotyl::unordered_map<cotyl::CString, Definition> definitions{};
  static const cotyl::unordered_map<cotyl::CString, cotyl::CString (Preprocessor::*)() const> StandardDefinitions;
  
  // default definition handlers
  cotyl::CString FILE() const;
  cotyl::CString LINE() const;
  cotyl::CString DATE() const;
  cotyl::CString TIME() const;
  cotyl::CString STDC() const;
  cotyl::CString STDC_HOSTED() const;
  cotyl::CString STDC_VERSION() const;

  // expand and push a new macro
  void PushMacro(cotyl::CString&& name, const Definition& definition);
  
  // get macro argument value, as well as the next delimiter
  // (either ','  or ')')
  std::pair<cotyl::CString, char> GetMacroArgumentValue(bool variadic);
  
  // expand macro argument to some StringStream by pushing
  // a new state and parsing an expression
  void ExpandArgumentTo(cotyl::StringStream& dest, const cotyl::CString& arg);
  
  // expand a macro definition and return the expanded value
  cotyl::CString ExpandMacro(const Definition& definition, cotyl::vector<cotyl::CString>&& args, cotyl::CString&& va_args);

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
   * GENERAL STATE MANAGEMENT
   * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  // wrapper around File cotyl::Stream, including
  // current line number and filename
  struct FileStream {
    FileStream(const std::string& name) : stream{name}, name{name} { }

    File stream;
    std::string name;
    u64 line = 1;  // line indexing starts at 1
  };

  // if group struct for tracking whether the current
  // group is even enabled
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
  
  // for saving / restoring state when preprocessing subexpressions
  struct State {
    // contains queued up characters to be processed
    // used to handle strings, identifiers and strings of alphanumerical characters
    std::queue<char> pre_processor_queue{};

    // current expression being parsed for #if condition
    std::optional<SString<std::string_view>> expression{};

    // stack of macros and arguments to be processed before anything else
    std::deque<Preprocessor::MacroStream> macro_stack{};
  };

  mutable State state{};

  // stack of included files to be processed
  mutable std::deque<FileStream> file_stack{};
  bool is_newline = true;

  // stack of if groups that may or may not be enabled
  std::deque<IfGroup> if_group_stack{};

  // a set of parsed files for tracking #pragma once directives
  cotyl::unordered_set<cotyl::CString> parsed_files{};

  // we want to reduce the "CurrentStream" usage as much as possible,
  // since it may introduce erroneous tracking of global preprocessor state
  // i.e. current newline status or line number
  // we comment on the use whenever we do use it, except in the wrapper
  // functions that handle the state updates properly (...NextCharacter)
  cotyl::Stream<char>& CurrentStream() const;

  // clears any empty streams on the macro / expression / file stacks
  void ClearEmptyStreams() const;

  // current line / file that is being read
  u64& CurrentLine();
  std::string CurrentFile();

  // checks whether all groups in the current if group
  // stack are enabled
  bool Enabled() const;

  // push / pop a new state to parse the passed expression
  // used for example to parse constant expressions for
  // #if / #line directives, or for expanding macros
  State StartExpression(const cotyl::CString& expr);
  void EndExpression(State&& old_state);


  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
   * DIRECTIVE HANDLING
   * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  // parse an integer valued constant expression
  // used for #if and #line directives
  i64 EatConstexpr();

  // parse an #include directive
  // parses the filename (possibly expanding macros)
  // and pushes a new file to the stack
  void Include();

  // find a file based on the filename parsed in the Include method
  // system includes (#include <...>) have slightly different
  // search paths
  std::string FindFile(const cotyl::CString& name, bool system);

  // handle preprocessing directive (preceded by #)
  void PreprocessorDirective();
};

}