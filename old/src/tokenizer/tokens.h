#ifndef EPICALYX_TOKENS_H
#define EPICALYX_TOKENS_H

#include <type_traits>
#include <string>
#include <vector>
#include <map>

#include "token_types.h"
#include "../state/state.h"
#define TOKEN std::shared_ptr<Token>
#define MAKE_TOKEN(...) std::make_shared<__VA_ARGS__>

class Token : public InFile {
public:
    explicit Token(INFILE_ARGS, TokenClass cls, enum TokenType type) :
        INFILE_CONSTRUCTOR,
        Class(cls),
        Type(type) {

    }

    static Token Keyword(INFILE_ARGS, enum TokenType type) {
        return Token(INFILE_VALUES, TokenClass::Keyword, type);
    }

    virtual std::string IdentifierName() const {
        throw std::runtime_error("Invalid token: expected identfier");
    }

    virtual std::string StringValue() const {
        throw std::runtime_error("Invalid token: expected string literal");
    }

    virtual std::string Repr() const {
        return TypeString(Type);
    }

    const TokenClass Class;
    const enum TokenType Type;

    static std::string ClassString(TokenClass cls) {
        return ClassMap.at(cls);
    }

    static std::string TypeString(enum TokenType type) {
        return TypeMap.at(type);
    }
private:
    static const std::map<TokenClass, std::string> ClassMap;
    static const std::map<enum TokenType, std::string> TypeMap;
};

class PunctuatorToken : public Token {
public:
    explicit PunctuatorToken(INFILE_ARGS, enum TokenType type, unsigned flags) :
        Token(INFILE_VALUES, TokenClass::Punctuator, type) ,
        Flags(flags) {

    }

    const unsigned Flags;
};

class StringConstantToken : public Token {
public:
    StringConstantToken(INFILE_ARGS, const std::string& value) :
        Token(INFILE_VALUES, TokenClass::StringConstant, TokenType::ConstString),
        Value(value) {

    }

    const std::string Value;
    std::string Repr() const override {
        return "\"" + Value + "\"";
    }

    std::string StringValue() const override {
        return Value;
    }

};

template<typename T>
class NumericalConstantToken : public Token {
public:
    NumericalConstantToken(INFILE_ARGS, enum TokenType type, const T& value) :
        Token(INFILE_VALUES, TokenClass::NumericalConstant, type),
        Value(value) {

    }

    const T Value;
    std::string Repr() const override {
        return std::to_string(Value);
    }
};


class IdentifierToken : public Token {
public:
    explicit IdentifierToken(INFILE_ARGS, const std::string& name) :
        Token(INFILE_VALUES, TokenClass::Identifier, TokenType::Identifier),
        Name(name) {

    }

    const std::string Name;
    std::string Repr() const override {
        return Name;
    }

    std::string IdentifierName() const override {
        return Name;
    }
};

#endif //EPICALYX_TOKENS_H
