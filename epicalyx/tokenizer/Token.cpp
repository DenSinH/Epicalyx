#include "Token.h"
#include "Format.h"


namespace epi {
  
extern const cotyl::unordered_map<TokenType, TokenInfo> TokenData;

STRINGIFY_METHOD(TokenType) {
  return TokenData.at(value).name;
}

}