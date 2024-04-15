#include "Token.h"
#include "Format.h"
#include "Escape.h"
#include "Containers.h"


namespace epi {
  
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
  return value.name; 
}

template<typename T>
STRINGIFY_METHOD(NumericalConstantToken<T>) { 
  return stringify(value.value); 
}

STRINGIFY_METHOD(StringConstantToken) {
  return cotyl::Format("\"%s\"", cotyl::Escape(value.value).c_str());
}

STRINGIFY_METHOD(AnyToken) {
  return value.visit<std::string>([](const auto& tok) -> std::string { 
    return stringify(tok); 
  });
}

}