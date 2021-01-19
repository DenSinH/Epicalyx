#ifndef EPICALYX_TOKENS_H
#define EPICALYX_TOKENS_H

#include <type_traits>
#include <string>
#include <vector>

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

private:
    static std::string ClassString(TokenClass cls) {
        switch(cls) {
            case TokenClass::StringConstant:
                return "StringConstant";
            case TokenClass::Identifier:
                return "Identifier";
            case TokenClass::Keyword:
                return "Keyword";
            case TokenClass::Punctuator:
                return "Punctuator";
            case TokenClass::NumericalConstant:
                return "NumericalConstant";
        }
    }

    static std::string TypeString(TokenType type) {
        switch(type) {
            case TokenType::ConstString:
                return "ConstString";
            case TokenType::Identifier:
                return "Identifier";
            case TokenType::ConstFloat:
                return "ConstFloat";
            case TokenType::ConstDouble:
                return "ConstDouble";
            case TokenType::ConstInt:
                return "ConstInt";
            case TokenType::ConstUnsignedInt:
                return "ConstUnsignedInt";
            case TokenType::ConstLong:
                return "ConstLong";
            case TokenType::ConstUnsignedLong:
                return "ConstUnsignedLong";
            case TokenType::ConstLongLong:
                return "ConstLongLong";
            case TokenType::ConstUnsignedLongLong:
                return "ConstUnsignedLongLong";
            case TokenType::Auto:
                return "auto";
            case TokenType::Break:
                return "break";
            case TokenType::Case:
                return "case";
            case TokenType::Char:
                return "char";
            case TokenType::Const:
                return "const";
            case TokenType::Continue:
                return "continue";
            case TokenType::Default:
                return "default";
            case TokenType::Do:
                return "do";
            case TokenType::Double:
                return "double";
            case TokenType::Else:
                return "else";
            case TokenType::Enum:
                return "enum";
            case TokenType::Extern:
                return "extern";
            case TokenType::Float:
                return "float";
            case TokenType::For:
                return "for";
            case TokenType::Goto:
                return "goto";
            case TokenType::If:
                return "if";
            case TokenType::Inline:
                return "inline";
            case TokenType::Int:
                return "int";
            case TokenType::Long:
                return "long";
            case TokenType::Register:
                return "register";
            case TokenType::Restrict:
                return "restrict";
            case TokenType::Return:
                return "return";
            case TokenType::Short:
                return "short";
            case TokenType::Signed:
                return "signed";
            case TokenType::Sizeof:
                return "sizeof";
            case TokenType::Static:
                return "static";
            case TokenType::Struct:
                return "struct";
            case TokenType::Switch:
                return "switch";
            case TokenType::Typedef:
                return "typedef";
            case TokenType::Union:
                return "union";
            case TokenType::Unsigned:
                return "unsigned";
            case TokenType::Void:
                return "void";
            case TokenType::Volatile:
                return "volatile";
            case TokenType::While:
                return "while";
            case TokenType::Alignas:
                return "_AlignAs";
            case TokenType::Alignof:
                return "_AlignOf";
            case TokenType::Atomic:
                return "_Atomic";
            case TokenType::Bool:
                return "_Bool";
            case TokenType::Complex:
                return "_Complex";
            case TokenType::Generic:
                return "_Generic";
            case TokenType::Imaginary:
                return "_Imaginary";
            case TokenType::Noreturn:
                return "_NoReturn";
            case TokenType::StaticAssert:
                return "_Static_assert";
            case TokenType::ThreadLocal:
                return "_Thread_local";
            case TokenType::LBracket:
                return "[";
            case TokenType::RBracket:
                return "]";
            case TokenType::LParen:
                return "(";
            case TokenType::RParen:
                return ")";
            case TokenType::LBrace:
                return "{";
            case TokenType::RBrace:
                return "}";
            case TokenType::Dot:
                return ".";
            case TokenType::Arrow:
                return "->";
            case TokenType::Incr:
                return "++";
            case TokenType::Decr:
                return "--";
            case TokenType::Ampersand:
                return "&";
            case TokenType::Asterisk:
                return "*";
            case TokenType::Plus:
                return "+";
            case TokenType::Minus:
                return "-";
            case TokenType::Tilde:
                return "~";
            case TokenType::Exclamation:
                return "!";
            case TokenType::Div:
                return "/";
            case TokenType::Mod:
                return "%";
            case TokenType::LShift:
                return "<<";
            case TokenType::RShift:
                return ">>";
            case TokenType::Less:
                return "<";
            case TokenType::Greater:
                return ">";
            case TokenType::LessEqual:
                return "<=";
            case TokenType::GreaterEqual:
                return ">=";
            case TokenType::Equal:
                return "==";
            case TokenType::NotEqual:
                return "!=";
            case TokenType::BinXor:
                return "^";
            case TokenType::BinOr:
                return "|";
            case TokenType::LogicalAnd:
                return "&&";
            case TokenType::LogicalOr:
                return "||";
            case TokenType::Question:
                return "?";
            case TokenType::Colon:
                return ":";
            case TokenType::SemiColon:
                return ";";
            case TokenType::Ellipsis:
                return "...";
            case TokenType::Assign:
                return "=";
            case TokenType::IMul:
                return "*=";
            case TokenType::IDiv:
                return "/=";
            case TokenType::IMod:
                return "%=";
            case TokenType::IPlus:
                return "+=";
            case TokenType::IMinus:
                return "-=";
            case TokenType::ILShift:
                return "<<=";
            case TokenType::IRShift:
                return ">>=";
            case TokenType::IAnd:
                return "&=";
            case TokenType::IXor:
                return "^=";
            case TokenType::IOr:
                return "|=";
            case TokenType::Comma:
                return ",";
            case TokenType::Hashtag:
                return "#";
            case TokenType::HHashtag:
                return "##";
        }
    }

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
