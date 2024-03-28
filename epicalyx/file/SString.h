#pragma once

#include "Stream.h"
#include <string>

namespace epi {

struct SString final : public cotyl::Stream<char> {

  SString(const std::string* string) : string(string) { };
  ~SString() = default;

  void PrintLoc() const final;

protected:
  char GetNew() final { return (*string)[position++]; }
  bool IsEOS() final { return position == string->length(); }

private:
  const std::string* string;
  int position = 0;
};

}