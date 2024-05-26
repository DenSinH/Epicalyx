#pragma once

#include <iosfwd>    // for ostream
#include "Stream.h"  // for Stream
#include "Token.h"   // for AnyToken

namespace epi { namespace cotyl { struct CString; } }  // lines 12-12

namespace epi {

class Tokenizer : public cotyl::Stream<AnyToken> {
public:

  Tokenizer(cotyl::Stream<char>& in_stream) :
      in_stream(in_stream) {

  }
  void PrintLoc(std::ostream& out) const final { 
    in_stream.PrintLoc(out); 
  };

protected:
  AnyToken GetNew() override;
  bool IsEOS() override;

  void SkipBlanks();

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
