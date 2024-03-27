#pragma once

#include "Stream.h"
#include <fstream>

namespace epi {

struct File final : public cotyl::Stream<char> {

  File(const std::string& filename);
  ~File();

  void PrintLoc() const final;

protected:
  char GetNew() final;
  bool IsEOS() final;

private:
  std::streampos prev = -1;
  std::streampos line = 0;
  
  // lookahead checking for null at the end of a file
  mutable char lookahead = 0;
  mutable std::ifstream file;
};

}