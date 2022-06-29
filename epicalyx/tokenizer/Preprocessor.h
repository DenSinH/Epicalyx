#pragma once

#include "Tokenizer.h"
#include "Containers.h"

#include "file/File.h"

#include <stack>


namespace epi {


struct Preprocessor final : public cotyl::Stream<char> {
  Preprocessor(const std::string& filename) {
     file_stack.emplace_back(filename);
  }

  void PrintLoc() const final;

protected:
  char GetNew() final;
  bool IsEOS() final;

private:
  struct PreprocessingFile {
    PreprocessingFile(const std::string& name) : stream{name}, name{name} { }

    File stream;
    std::string name;
    int line = 0;
    bool is_newline = true;
  };

  struct Definition {

    std::vector<std::string> arguments;
  };

  std::queue<char> pre_processor_queue{};
  std::deque<PreprocessingFile> file_stack{};
  cotyl::unordered_map<std::string, std::vector<pToken>> definitions{};
  cotyl::unordered_set<std::string> parsed_files{};

  File& CurrentStream();
  bool IsNewline();
  void SetNewline(bool status);
  void SkipBlanks(bool skip_newlines = true);
  void ExpectNewline();
  char NextCharacter();
  std::string ExpectIdentifier();

  void PreprocessorDirective();
};

}