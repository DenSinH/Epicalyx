#pragma once

#include "Tokenizer.h"

#include "file/File.h"
#include "file/SString.h"

#include "Containers.h"

#include <stack>
#include <utility>


namespace epi {


struct Preprocessor final : public cotyl::Stream<char> {
  Preprocessor(const std::string& in_stream) {
    file_stack.emplace_back(in_stream);
  }

  void PrintLoc() const final;

protected:
  char GetNew() final;
  bool IsEOS() final;

private:
  struct Definition;
  using MacroMap = cotyl::unordered_map<std::string, Definition>;

  struct FileStream {
    FileStream(const std::string& name) : stream{name}, name{name} { }

    File stream;
    std::string name;
    u64 line = 0;
    bool is_newline = true;
  };

  struct Definition {
    struct Arguments {
      bool variadic;
      std::vector<std::string> list;
    };

    std::string value;
    std::optional<Arguments> arguments{};
  };

  struct MacroStream {
    MacroStream(std::string name, const std::string& s) : name{std::move(name)}, stream{s} { }
    MacroStream(std::string name, const std::string& s, MacroMap&& arguments) :
        name{std::move(name)},
        stream{s},
        arguments{std::move(arguments)} {

    }

    std::string name;
    SString stream;
    MacroMap arguments{};
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
        throw std::runtime_error("Unexpected elif group");
      }

      // only enable group if no earlier chain was taken and the condition is true
      enabled = !was_enabled && cond;
      was_enabled |= cond;
      type = Type::Elif;
    }

    void Else() {
      if (type == Type::Else) {
        throw std::runtime_error("Unexpected else group");
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
  std::optional<SString> expression{};

  // stack of macros and arguments to be processed before anything else
  std::deque<MacroStream> macro_stack{};

  // stack of included files to be processed
  std::deque<FileStream> file_stack{};

  // stack of if groups that may or may not be enabled
  std::deque<IfGroup> if_group_stack{};
  MacroMap definitions{};
  cotyl::unordered_set<std::string> parsed_files{};

  cotyl::Stream<char>& CurrentStream();
  u64& CurrentLine();
  bool& IsNewline();
  std::string CurrentFile();

  bool Enabled() const;

  void SkipBlanks(bool skip_newlines = true);
  void SkipLineComment();
  void SkipMultilineComment();
  std::string FetchLine();
  char NextCharacter();

  enum class MacroExpansion {
    Normal, Argument
  };

  std::string GetNextProcessed(MacroExpansion macro_expansion = MacroExpansion::Normal);

  std::string GetMacroArgumentValue(bool variadic);
  bool IsDefinition(const std::string& name, Definition& definition);
  void PushMacro(const std::string& name, const Definition& definition);
  void EatNewline();
  i64 EatConstexpr();

  void PreprocessorDirective();
};

}