#pragma once

#include "Stream.h"
#include <fstream>

namespace epi {

struct File final : public cotyl::Stream<char> {

  File(std::string filename);
  ~File();


protected:
  char GetNew() final;
  bool IsEOS() final;

private:
  std::ifstream file;
};

}