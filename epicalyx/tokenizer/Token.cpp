#include "Token.h"
#include <string>        // for string
#include "Containers.h"  // for unordered_map
#include "Escape.h"      // for QuotedEscape
#include "SStream.h"     // for StringStream


namespace epi {

template<> detail::any_token_t::~Variant() = default;

extern const cotyl::unordered_map<TokenType, std::string> TokenNames;

STRINGIFY_METHOD(TokenType) {
  return TokenNames.at(value);
}

STRINGIFY_METHOD(PunctuatorToken) { 
  return stringify(value.type); 
}

STRINGIFY_METHOD(KeywordToken) { 
  return stringify(value.type); 
}

STRINGIFY_METHOD(IdentifierToken) { 
  return std::string(value.name.str()); 
}

template<typename T>
STRINGIFY_METHOD(NumericalConstantToken<T>) { 
  return stringify(value.value); 
}

STRINGIFY_METHOD(StringConstantToken) {
  return cotyl::QuotedEscape(value.value.c_str()).finalize();
}

STRINGIFY_METHOD(AnyToken) {
  return value.visit<std::string>([](const auto& tok) -> std::string { 
    return stringify(tok); 
  });
}

}