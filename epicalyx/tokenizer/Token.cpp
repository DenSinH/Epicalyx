#include "Token.h"
#include "Format.h"


namespace epi {
  
extern const cotyl::unordered_map<TokenType, std::string> TokenNames;

STRINGIFY_METHOD(TokenType) {
  return TokenNames.at(value);
}

}