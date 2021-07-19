#include "Token.h"
#include "Format.h"


namespace epi {

void Token::Expect(TokenType t) const {
  if (type != t) [[unlikely]] {
    throw cotyl::FormatExcept("Invalid token: expected %s, got %s", TokenData.at(t).name.c_str(), TokenData.at(type).name.c_str());
  }
}

}