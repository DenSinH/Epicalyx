#pragma once

#include "Tokenizer.h"
#include "Containers.h"

#include "file/File.h"
#include "file/SString.h"

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
  friend struct LinePreprocessor;

  struct FileStream {
    FileStream(const std::string& name) : stream{name}, name{name} { }

    File stream;
    std::string name;
    u64 line = 0;
    bool is_newline = true;
  };

  struct StringStream {
    StringStream(std::string name, const std::string& s) : name{std::move(name)}, stream{s} { }

    std::string name;
    SString stream;
  };

  struct Definition {
    struct Arguments {
      bool variadic;
      std::vector<std::string> list;
    };

    std::string value;
    std::optional<Arguments> arguments{};
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

  std::queue<char> pre_processor_queue{};
  std::optional<SString> expression{};
  std::deque<StringStream> macro_stack{};
  std::deque<FileStream> file_stack{};
  std::deque<IfGroup> if_group_stack{};
  cotyl::unordered_map<std::string, Definition> definitions{};
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

  void PushMacro(const std::string& macro);
  void EatNewline();
  i64 EatConstexpr();

  void PreprocessorDirective();
};

}