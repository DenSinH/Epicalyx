#ifndef EPICALYX_TOKENS_H
#define EPICALYX_TOKENS_H

#include <type_traits>

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

template<typename T>
class Number : public Token {
public:
    Number(TokenClass cls, TokenType type, T value) : Token(cls, Type) {
        this->Value = value;
    }

    T Value;
};

#endif //EPICALYX_TOKENS_H
