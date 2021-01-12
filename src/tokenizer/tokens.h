#ifndef EPICALYX_TOKENS_H
#define EPICALYX_TOKENS_H

#include <type_traits>
#include <string>

#include "types.h"

class Token {
public:
    explicit Token(TokenClass cls, TokenType type) {
        this->Class = cls;
        this->Type = type;
    }

    static Token Punctuator(TokenType type) {
        return Token(TokenClass::Punctuator, type);
    }

    static Token Keyword(TokenType type) {
        return Token(TokenClass::Keyword, type);
    }

    TokenClass Class;
    TokenType Type;
};

class StringConstant : public Token {
public:
    StringConstant(const std::string& value) : Token(TokenClass::StringConstant, TokenType::ConstString) {
        this->Value = value;
    }

    std::string Value;
};

class CharStringConstant : public Token {
public:
    CharStringConstant(const std::string& value) : Token(TokenClass::CharStringConstant, TokenType::ConstCharString) {
        this->Value = value;
    }

    std::string Value;
};

template<typename T>
class NumericalConstant : public Token {
public:
    NumericalConstant(TokenType type, const T& value) : Token(TokenClass::NumericalConstant, type){
        this->Value = value;
    }

    T Value;
};


class Identifier : public Token {
public:
    explicit Identifier(std::string& name) : Token(TokenClass::Identifier, TokenType::Identifier) {
        this->Name = name;
    }

    std::string Name;
};

#endif //EPICALYX_TOKENS_H
