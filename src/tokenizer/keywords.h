#ifndef EPICALYX_KEYWORDS_H
#define EPICALYX_KEYWORDS_H

#include "tokens.h"
#include <map>
#include <string>

static const std::map<const std::string, TokenType> Keywords = {
        { "auto",           TokenType::Auto },
        { "break",          TokenType::Break },
        { "case",           TokenType::Case },
        { "char",           TokenType::Char },
        { "const",          TokenType::Const },
        { "continue",       TokenType::Continue },
        { "default",        TokenType::Default },
        { "do",             TokenType::Do },
        { "double",         TokenType::Double },
        { "else",           TokenType::Else },
        { "enum",           TokenType::Enum },
        { "extern",         TokenType::Extern },
        { "float",          TokenType::Float },
        { "for",            TokenType::For },
        { "goto",           TokenType::Goto },
        { "if",             TokenType::If },
        { "inline",         TokenType::Inline },
        { "int",            TokenType::Int },
        { "long",           TokenType::Long },
        { "register",       TokenType::Register },
        { "restrict",       TokenType::Restrict },
        { "return",         TokenType::Return },
        { "short",          TokenType::Short },
        { "signed",         TokenType::Signed },
        { "sizeof",         TokenType::Sizeof },
        { "static",         TokenType::Static },
        { "struct",         TokenType::Struct },
        { "switch",         TokenType::Switch },
        { "typedef",        TokenType::Typedef },
        { "union",          TokenType::Union },
        { "unsigned",       TokenType::Unsigned },
        { "void",           TokenType::Void },
        { "volatile",       TokenType::Volatile },
        { "while",          TokenType::While },
        { "_Alignas",       TokenType::Alignas },
        { "_Alignof",       TokenType::Alignof },
        { "_Atomic",        TokenType::Atomic },
        { "_Bool",          TokenType::Bool },
        { "_Complex",       TokenType::Complex },
        { "_Generic",       TokenType::Generic },
        { "_Imaginary",     TokenType::Imaginary },
        { "_Noreturn",      TokenType::Noreturn },
        { "_Static_assert", TokenType::StaticAssert },
        { "_Thread_local",  TokenType::ThreadLocal },
};

#endif //EPICALYX_KEYWORDS_H
