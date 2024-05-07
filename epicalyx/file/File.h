#pragma once

#include "Stream.h"
#include <fstream>

namespace epi {

struct File : public cotyl::Stream<char> {

  File(File&& other) = default;
  File(const std::string& filename);
  ~File();

  void PrintLoc(std::ostream& out) const final;

  u64 lineno = 1;

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