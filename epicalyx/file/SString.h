#pragma once

#include "Stream.h"
#include <string>

namespace epi {

struct SString final : public cotyl::Stream<char> {

  SString(std::string_view&& string) : string(std::move(string)) { };
  ~SString() = default;

  void PrintLoc(std::ostream& out) const final;

protected:
  char GetNew() final { return (string)[position++]; }
  bool IsEOS() final { return position == string.length(); }

private:
  std::string_view string;
  int position = 0;
};

}