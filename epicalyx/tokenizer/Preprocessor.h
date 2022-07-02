#pragma once

#include "Tokenizer.h"
#include "Containers.h"

#include "file/File.h"

#include <stack>


namespace epi {


struct Preprocessor final : public cotyl::Stream<char> {
  Preprocessor(cotyl::Stream<char>& in_stream) : in_stream{in_stream} { }

  void PrintLoc() const final;

protected:
  char GetNew() final;
  bool IsEOS() final;

private:
  struct BaseStream {
    cotyl::Stream<char>& stream;
    int line = 0;
    bool is_newline = true;
  };

  struct IncludeStream {
    IncludeStream(const std::string& name) : stream{name}, name{name} { }

    File stream;
    std::string name;
    int line = 0;
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
  BaseStream in_stream;
  std::deque<IncludeStream> include_stack{};
  std::deque<IfGroup> if_group_stack{};
  cotyl::unordered_map<std::string, Definition> definitions{};
  cotyl::unordered_set<std::string> parsed_files{};

  cotyl::Stream<char>& CurrentStream();
  int& CurrentLine();
  bool IsNewline() const;
  bool Enabled() const;
  void SetNewline(bool status);

  void SkipBlanks(bool skip_newlines = true);
  std::string FetchLine();
  char NextCharacter();

  void EatNewline();
  std::string EatIdentifier();
  i64 EatConstexpr();

  void PreprocessorDirective();
};

}