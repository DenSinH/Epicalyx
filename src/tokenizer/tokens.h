#ifndef EPICALYX_TOKENS_H
#define EPICALYX_TOKENS_H

#include <type_traits>

#include "types.h"

class Token {
public:
    explicit Token(TokenClass cls, TokenType type) {
        this->Class  = cls;
        this->Type = type;
    }

    TokenClass Class;
    TokenType Type;
};

class Punctuator : Token {
public:
    explicit Punctuator(TokenType type) : Token(TokenClass::Punctuator, type) {

    }
};

class Keyword : Token {
public:
    explicit Keyword(TokenType type) : Token(TokenClass::Keyword, type) {

    }
};

template<typename T>
class Number : Token {
public:
    Number(TokenClass cls, TokenType type, T value) : Token(cls, Type) {
        this->Value = value;
    }

    T Value;
};

#endif //EPICALYX_TOKENS_H
