#pragma once

#include "Stream.h"
#include "Token.h"

#include <memory>
#include <queue>

namespace epi {

struct AnyToken;

namespace cotyl {

struct CString;

}

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
  static AnyToken Make(Args&&... args);

  cotyl::CString ReadString(const char delimiter);
  AnyToken ReadNumericalConstant();
};

}
