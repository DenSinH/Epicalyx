#include "ConstParser.h"
#include "tokenizer/Token.h"
#include "Stream.h"


namespace epi {

ConstParser::ConstParser(cotyl::Stream<AnyToken>& in_stream) : 
    in_stream{in_stream} { }


void ConstParser::PrintLoc(std::ostream& out) const {
  in_stream.PrintLoc(out);
}

// expression methods are in Parser.Expressions.cpp,
// since these are more related to general parsing

}
