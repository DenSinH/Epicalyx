#ifndef EPICALYX_TOKENS_H
#define EPICALYX_TOKENS_H

#include <type_traits>
#include <string>
#include <vector>
#include <map>

#include "types.h"
#define TOKEN std::shared_ptr<Token>
#define MAKE_TOKEN(_type) std::make_shared<_type>

class Token {
public:
    explicit Token(TokenClass cls, TokenType type) {
        this->Class = cls;
        this->Type = type;
    }

    static Token Keyword(TokenType type) {
        return Token(TokenClass::Keyword, type);
    }

    virtual std::string Repr() {
        return ClassString(Class) + " : " + TypeString(Type);
    }
    TokenClass Class;
    TokenType Type;

    static std::string ClassString(TokenClass cls) {
        return ClassMap.at(cls);
    }

    static std::string TypeString(TokenType type) {
        return TypeMap.at(type);
    }
private:
    static const std::map<TokenClass, std::string> ClassMap;
    static const std::map<TokenType, std::string> TypeMap;
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
    std::string Repr() override {
        return "String literal: " + Value;
    }
};

template<typename T>
class NumericalConstant : public Token {
public:
    NumericalConstant(TokenType type, const T& value) : Token(TokenClass::NumericalConstant, type){
        this->Value = value;
    }

    T Value;
    std::string Repr() override {
        return "Constant: " + std::to_string(Value);
    }
};


class Identifier : public Token {
public:
    explicit Identifier(std::string& name) : Token(TokenClass::Identifier, TokenType::Identifier) {
        this->Name = name;
    }

    std::string Name;
    std::string Repr() override {
        return "Identifier: " + Name;
    }
};

#endif //EPICALYX_TOKENS_H
