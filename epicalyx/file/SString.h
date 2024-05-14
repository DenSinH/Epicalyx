#pragma once

#include "Stream.h"
#include "CString.h"

namespace epi {

template<typename T>
struct SString : public cotyl::Stream<char> {
  static SString Empty;

  SString(T&& string) : string(std::move(string)) { };

  void PrintLoc(std::ostream& out) const override {
    std::size_t error_pos = position - BufSize();
    const auto view = (std::string_view)string;
    out << "..." << view.substr(std::max<std::size_t>(0ull, error_pos - 20), 40) << "..." << std::endl;
    for (std::size_t i = 0; i < 3 + std::min<std::size_t>(error_pos, 20ull) - 1; i++) {
      // - 1 because we are already advanced past the error the moment we catch it
      out << ' ';
    }
    out << '^' << std::endl;
  }

protected:
  char GetNew() final { return string[position++]; }
  bool IsEOS() final { return position == string.size(); }

private:
  T string;
  int position = 0;
};

}