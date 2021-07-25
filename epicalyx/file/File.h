#pragma once

#include "Stream.h"
#include <fstream>

namespace epi {

struct File final : public cotyl::Stream<char>, cotyl::Locatable {

  File(std::string filename);
  ~File();

  void PrintLoc() final;

protected:
  char GetNew() final;
  bool IsEOS() final;

private:
  std::streampos prev = -1;
  std::streampos line = 0;
  std::ifstream file;
};

}