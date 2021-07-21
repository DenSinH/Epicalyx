#pragma once

#include "Stream.h"
#include <string>

namespace epi {

struct SString final : public cotyl::Stream<char> {

  SString(std::string string) : string(std::move(string)) { };
  ~SString() = default;


protected:
  char GetNew() final { return string[position++]; }
  bool IsEOS() final { return position == string.length(); }

private:
  std::string string;
  int position = 0;
};

}