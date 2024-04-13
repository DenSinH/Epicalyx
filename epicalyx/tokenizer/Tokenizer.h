#pragma once

#include "Stream.h"
#include "Token.h"

#include <memory>
#include <queue>

namespace epi {

class Tokenizer : public cotyl::Stream<AnyToken> {
public:

  Tokenizer(cotyl::Stream<char>& in_stream) :
      in_stream(in_stream) {

  }
  void PrintLoc() const final { in_stream.PrintLoc(); };

protected:
  AnyToken GetNew() override;
  bool IsEOS() override;

  virtual void SkipBlanks();

  cotyl::Stream<char>& in_stream;

private:
  template<typename T, typename ...Args>
  static AnyToken Make(Args... args) {
    return AnyToken::Make<T>(args...);
  }

  std::string ReadString(const char delimiter);
  AnyToken ReadNumericalConstant();
};

}
