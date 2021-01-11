#ifndef EPICALYX_KEYWORDS_H
#define EPICALYX_KEYWORDS_H

#include "tokens.h"
#include <map>
#include <string>

static const std::map<const std::string, const Token> Keywords = {
        { "auto",           Token::Keyword(TokenType::Auto) },
        { "break",          Token::Keyword(TokenType::Break) },
        { "case",           Token::Keyword(TokenType::Case) },
        { "char",           Token::Keyword(TokenType::Char) },
        { "const",          Token::Keyword(TokenType::Const) },
        { "continue",       Token::Keyword(TokenType::Continue) },
        { "default",        Token::Keyword(TokenType::Default) },
        { "do",             Token::Keyword(TokenType::Do) },
        { "double",         Token::Keyword(TokenType::Double) },
        { "else",           Token::Keyword(TokenType::Else) },
        { "enum",           Token::Keyword(TokenType::Enum) },
        { "extern",         Token::Keyword(TokenType::Extern) },
        { "float",          Token::Keyword(TokenType::Float) },
        { "for",            Token::Keyword(TokenType::For) },
        { "goto",           Token::Keyword(TokenType::Goto) },
        { "if",             Token::Keyword(TokenType::If) },
        { "inline",         Token::Keyword(TokenType::Inline) },
        { "int",            Token::Keyword(TokenType::Int) },
        { "long",           Token::Keyword(TokenType::Long) },
        { "register",       Token::Keyword(TokenType::Register) },
        { "restrict",       Token::Keyword(TokenType::Restrict) },
        { "return",         Token::Keyword(TokenType::Return) },
        { "short",          Token::Keyword(TokenType::Short) },
        { "signed",         Token::Keyword(TokenType::Signed) },
        { "sizeof",         Token::Keyword(TokenType::Sizeof) },
        { "static",         Token::Keyword(TokenType::Static) },
        { "struct",         Token::Keyword(TokenType::Struct) },
        { "switch",         Token::Keyword(TokenType::Switch) },
        { "typedef",        Token::Keyword(TokenType::Typedef) },
        { "union",          Token::Keyword(TokenType::Union) },
        { "unsigned",       Token::Keyword(TokenType::Unsigned) },
        { "void",           Token::Keyword(TokenType::Void) },
        { "volatile",       Token::Keyword(TokenType::Volatile) },
        { "while",          Token::Keyword(TokenType::While) },
        { "_Alignas",       Token::Keyword(TokenType::Alignas) },
        { "_Alignof",       Token::Keyword(TokenType::Alignof) },
        { "_Atomic",        Token::Keyword(TokenType::Atomic) },
        { "_Bool",          Token::Keyword(TokenType::Bool) },
        { "_Complex",       Token::Keyword(TokenType::Complex) },
        { "_Generic",       Token::Keyword(TokenType::Generic) },
        { "_Imaginary",     Token::Keyword(TokenType::Imaginary) },
        { "_Noreturn",      Token::Keyword(TokenType::Noreturn) },
        { "_Static_assert", Token::Keyword(TokenType::StaticAssert) },
        { "_Thread_local",  Token::Keyword(TokenType::ThreadLocal) },
};

#endif //EPICALYX_KEYWORDS_H
