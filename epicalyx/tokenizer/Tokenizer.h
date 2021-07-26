#pragma once

#include "Stream.h"
#include "Token.h"

#include <memory>

namespace epi {

class Tokenizer : public cotyl::Stream<std::unique_ptr<Token>> {
public:

  Tokenizer(cotyl::Stream<char>& in_stream) :
      in_stream(in_stream) {

  }

protected:
  pToken GetNew() final;
  bool IsEOS() final;


private:
  cotyl::Stream<char>& in_stream;

  void SkipBlanks();

  template<typename T, typename ...Args>
  static pToken Make(Args... args) {
    return std::make_unique<T>(args...);
  }

  std::string ReadString(const char delimiter);
  pToken ReadNumericalConstant();
};

}
