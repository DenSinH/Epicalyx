#pragma once

#include "Stream.h"
#include "Token.h"

#include <memory>
#include <queue>

namespace epi {

class Tokenizer : public cotyl::Stream<std::unique_ptr<Token>> {
public:

  Tokenizer(cotyl::Stream<char>& in_stream) :
      in_stream(in_stream) {

  }
  void PrintLoc() const final { in_stream.PrintLoc(); };

protected:
  pToken GetNew() override;
  bool IsEOS() override;

  virtual void SkipBlanks();

  cotyl::Stream<char>& in_stream;

private:
  template<typename T, typename ...Args>
  static pToken Make(Args... args) {
    return std::make_unique<T>(args...);
  }

  std::string ReadString(const char delimiter);
  pToken ReadNumericalConstant();
};

}
