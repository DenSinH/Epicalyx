#include "Token.h"
#include "Format.h"
#include "Escape.h"
#include "Containers.h"


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
  cotyl::StringStream escaped{};
  cotyl::QuotedEscapeTo(escaped, value.value.c_str());
  return escaped.finalize();
}

STRINGIFY_METHOD(AnyToken) {
  return value.visit<std::string>([](const auto& tok) -> std::string { 
    return stringify(tok); 
  });
}

}