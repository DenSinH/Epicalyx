#include "Token.h"
#include "Format.h"


namespace epi {

void Token::Expect(TokenType t) const {
  if (type != t) [[unlikely]] {
    throw cotyl::FormatExceptStr("Invalid token: expected %s, got %s", Token(t), *this);
  }
}

}