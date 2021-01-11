#ifndef EPICALYX_PUNCTUATORS_H
#define EPICALYX_PUNCTUATORS_H

#include "tokens.h"

#include <map>
#include <string>

static const std::map<const std::string, const Punctuator> Punctuators = {
        { "[",   Punctuator(TokenType::LBracket) },
        { "]",   Punctuator(TokenType::RBracket) },
        { "(",   Punctuator(TokenType::LParen) },
        { ")",   Punctuator(TokenType::RParen) },
        { "{",   Punctuator(TokenType::LBrace) },
        { "}",   Punctuator(TokenType::RBrace) },
        { ".",   Punctuator(TokenType::Dot) },
        { "->",  Punctuator(TokenType::Arrow) },
        { "++",  Punctuator(TokenType::Incr) },
        { "--",  Punctuator(TokenType::Decr) },
        { "&",   Punctuator(TokenType::Ampersand) },
        { "*",   Punctuator(TokenType::Asterisk) },
        { "+",   Punctuator(TokenType::Plus) },
        { "-",   Punctuator(TokenType::Minus) },
        { "~",   Punctuator(TokenType::Tilde) },
        { "!",   Punctuator(TokenType::Exclamation) },
        { "/",   Punctuator(TokenType::Div) },
        { "%",   Punctuator(TokenType::Mod) },
        { "<<",  Punctuator(TokenType::LShift) },
        { ">>",  Punctuator(TokenType::RShift) },
        { "<",   Punctuator(TokenType::Less) },
        { ">",   Punctuator(TokenType::Greater) },
        { "<=",  Punctuator(TokenType::LessEqual) },
        { ">=",  Punctuator(TokenType::GreaterEqual) },
        { "==",  Punctuator(TokenType::Equal) },
        { "!=",  Punctuator(TokenType::NotEqual) },
        { "^",   Punctuator(TokenType::BinXor) },
        { "|",   Punctuator(TokenType::BinOr) },
        { "&&",  Punctuator(TokenType::LogicalAnd) },
        { "||",  Punctuator(TokenType::LogicalOr) },
        { "?",   Punctuator(TokenType::Question) },
        { ":",   Punctuator(TokenType::Colon) },
        { ";",   Punctuator(TokenType::SemiColon) },
        { "...", Punctuator(TokenType::Ellipsis) },
        { "=",   Punctuator(TokenType::Assign) },
        { "*=",  Punctuator(TokenType::IMul) },
        { "/=",  Punctuator(TokenType::IDiv) },
        { "%=",  Punctuator(TokenType::IMod) },
        { "+=",  Punctuator(TokenType::IPlus) },
        { "-=",  Punctuator(TokenType::IMinus) },
        { "<<=", Punctuator(TokenType::ILShift) },
        { ">>=", Punctuator(TokenType::IRShift) },
        { "&=",  Punctuator(TokenType::IAnd) },
        { "^=",  Punctuator(TokenType::IXor) },
        { "|=",  Punctuator(TokenType::IOr) },
        { ",",   Punctuator(TokenType::Comma) },
        { "#",   Punctuator(TokenType::Hashtag) },
        { "##",  Punctuator(TokenType::HHashtag) },
};

#endif //EPICALYX_PUNCTUATORS_H
