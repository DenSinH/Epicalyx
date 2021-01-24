#ifndef EPICALYX_PUNCTUATORS_H
#define EPICALYX_PUNCTUATORS_H

#include "tokens.h"

#include <map>
#include <string>


static const std::map<const std::string, TokenType> Punctuators = {
        { "[",   TokenType::LBracket },
        { "]",   TokenType::RBracket },
        { "(",   TokenType::LParen },
        { ")",   TokenType::RParen },
        { "{",   TokenType::LBrace },
        { "}",   TokenType::RBrace },
        { ".",   TokenType::Dot },
        { "->",  TokenType::Arrow },
        { "++",  TokenType::Incr },
        { "--",  TokenType::Decr },
        { "&",   TokenType::Ampersand },
        { "*",   TokenType::Asterisk },
        { "+",   TokenType::Plus },
        { "-",   TokenType::Minus },
        { "~",   TokenType::Tilde },
        { "!",   TokenType::Exclamation },
        { "/",   TokenType::Div },
        { "%",   TokenType::Mod },
        { "<<",  TokenType::LShift },
        { ">>",  TokenType::RShift },
        { "<",   TokenType::Less },
        { ">",   TokenType::Greater },
        { "<=",  TokenType::LessEqual },
        { ">=",  TokenType::GreaterEqual },
        { "==",  TokenType::Equal },
        { "!=",  TokenType::NotEqual },
        { "^",   TokenType::BinXor },
        { "|",   TokenType::BinOr },
        { "&&",  TokenType::LogicalAnd },
        { "||",  TokenType::LogicalOr },
        { "?",   TokenType::Question },
        { ":",   TokenType::Colon },
        { ";",   TokenType::SemiColon },
        { "...", TokenType::Ellipsis },
        { "=",   TokenType::Assign },
        { "*=",  TokenType::IMul },
        { "/=",  TokenType::IDiv },
        { "%=",  TokenType::IMod },
        { "+=",  TokenType::IPlus },
        { "-=",  TokenType::IMinus },
        { "<<=", TokenType::ILShift },
        { ">>=", TokenType::IRShift },
        { "&=",  TokenType::IAnd },
        { "^=",  TokenType::IXor },
        { "|=",  TokenType::IOr },
        { ",",   TokenType::Comma },
        { "#",   TokenType::Hashtag },
        { "##",  TokenType::HHashtag },
};

#endif //EPICALYX_PUNCTUATORS_H
