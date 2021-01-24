#ifndef EPICALYX_TOKENS_H
#define EPICALYX_TOKENS_H

#include <type_traits>
#include <string>
#include <vector>
#include <map>

#include "types.h"
#include "../state/state.h"
#define TOKEN std::shared_ptr<Token>
#define MAKE_TOKEN(_type) std::make_shared<_type>

class Token : public InFile {
public:
    explicit Token(INFILE_ARGS, TokenClass cls, TokenType type) :
        INFILE_CONSTRUCTOR,
        Class(cls),
        Type(type) {

    }

    static Token Keyword(INFILE_ARGS, TokenType type) {
        return Token(INFILE_VALUES, TokenClass::Keyword, type);
    }

    virtual std::string Repr() {
        return ClassString(Class) + " : " + TypeString(Type);
    }
    const TokenClass Class;
    const TokenType Type;

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
    explicit Punctuator(INFILE_ARGS, TokenType type, unsigned flags) :
        Token(INFILE_VALUES, TokenClass::Punctuator, type) ,
        Flags(flags) {

    }

    const unsigned Flags;
};

class StringConstant : public Token {
public:
    StringConstant(INFILE_ARGS, const std::string& value) :
        Token(INFILE_VALUES, TokenClass::StringConstant, TokenType::ConstString),
        Value(value) {

    }

    const std::string Value;
    std::string Repr() override {
        return "String literal: " + Value;
    }
};

template<typename T>
class NumericalConstant : public Token {
public:
    NumericalConstant(INFILE_ARGS, TokenType type, const T& value) :
        Token(INFILE_VALUES, TokenClass::NumericalConstant, type),
        Value(value) {

    }

    const T Value;
    std::string Repr() override {
        return "Constant: " + std::to_string(Value);
    }
};


class Identifier : public Token {
public:
    explicit Identifier(INFILE_ARGS, const std::string& name) :
        Token(INFILE_VALUES, TokenClass::Identifier, TokenType::Identifier),
        Name(name) {

    }

    const std::string Name;
    std::string Repr() override {
        return "Identifier: " + Name;
    }
};

#endif //EPICALYX_TOKENS_H
