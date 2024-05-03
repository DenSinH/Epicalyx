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
  void PrintLoc(std::ostream& out) const final { in_stream.PrintLoc(out); };

protected:
  AnyToken GetNew() override;
  bool IsEOS() override;

  virtual void SkipBlanks();

  cotyl::Stream<char>& in_stream;

private:
  // for accessing ReadString
  friend struct Preprocessor;

  template<typename T, typename ...Args>
  static AnyToken Make(Args&&... args);

  cotyl::CString ReadString(char delimiter = 0);
  AnyToken ReadNumericalConstant();
};

}
