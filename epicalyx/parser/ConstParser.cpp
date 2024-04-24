#include "ConstParser.h"
#include "tokenizer/Token.h"
#include "Stream.h"


namespace epi {

ConstParser::ConstParser(cotyl::Stream<AnyToken>& in_stream) : 
    in_stream{in_stream} { }


void ConstParser::PrintLoc() const {
  in_stream.PrintLoc();
}

// expression methods are in Parser.Expressions.cpp,
// since these are more related to general parsing

}
