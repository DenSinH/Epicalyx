#ifndef EPICALYX_TOKENS_H
#define EPICALYX_TOKENS_H

#include <type_traits>
#include <string>
#include <vector>

#include "types.h"

class Token {
public:
    explicit Token(TokenClass cls, TokenType type) {
        this->Class = cls;
        this->Type = type;
    }

    static Token Keyword(TokenType type) {
        return Token(TokenClass::Keyword, type);
    }

    TokenClass Class;
    TokenType Type;
};

class Punctuator : public Token {
public:
    explicit Punctuator(TokenType type, unsigned flags) : Token(TokenClass::Punctuator, type) {
        this->Flags = flags;
    }

    unsigned Flags;
};

class StringConstant : public Token {
public:
    StringConstant(const std::string& value) : Token(TokenClass::StringConstant, TokenType::ConstString) {
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
