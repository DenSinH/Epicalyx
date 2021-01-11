#ifndef EPICALYX_PUNCTUATORS_H
#define EPICALYX_PUNCTUATORS_H

#include "tokens.h"

#include <map>
#include <string>

static const std::map<const std::string, const Token> Punctuators = {
        { "[",   Token::Punctuator(TokenType::LBracket) },
        { "]",   Token::Punctuator(TokenType::RBracket) },
        { "(",   Token::Punctuator(TokenType::LParen) },
        { ")",   Token::Punctuator(TokenType::RParen) },
        { "{",   Token::Punctuator(TokenType::LBrace) },
        { "}",   Token::Punctuator(TokenType::RBrace) },
        { ".",   Token::Punctuator(TokenType::Dot) },
        { "->",  Token::Punctuator(TokenType::Arrow) },
        { "++",  Token::Punctuator(TokenType::Incr) },
        { "--",  Token::Punctuator(TokenType::Decr) },
        { "&",   Token::Punctuator(TokenType::Ampersand) },
        { "*",   Token::Punctuator(TokenType::Asterisk) },
        { "+",   Token::Punctuator(TokenType::Plus) },
        { "-",   Token::Punctuator(TokenType::Minus) },
        { "~",   Token::Punctuator(TokenType::Tilde) },
        { "!",   Token::Punctuator(TokenType::Exclamation) },
        { "/",   Token::Punctuator(TokenType::Div) },
        { "%",   Token::Punctuator(TokenType::Mod) },
        { "<<",  Token::Punctuator(TokenType::LShift) },
        { ">>",  Token::Punctuator(TokenType::RShift) },
        { "<",   Token::Punctuator(TokenType::Less) },
        { ">",   Token::Punctuator(TokenType::Greater) },
        { "<=",  Token::Punctuator(TokenType::LessEqual) },
        { ">=",  Token::Punctuator(TokenType::GreaterEqual) },
        { "==",  Token::Punctuator(TokenType::Equal) },
        { "!=",  Token::Punctuator(TokenType::NotEqual) },
        { "^",   Token::Punctuator(TokenType::BinXor) },
        { "|",   Token::Punctuator(TokenType::BinOr) },
        { "&&",  Token::Punctuator(TokenType::LogicalAnd) },
        { "||",  Token::Punctuator(TokenType::LogicalOr) },
        { "?",   Token::Punctuator(TokenType::Question) },
        { ":",   Token::Punctuator(TokenType::Colon) },
        { ";",   Token::Punctuator(TokenType::SemiColon) },
        { "...", Token::Punctuator(TokenType::Ellipsis) },
        { "=",   Token::Punctuator(TokenType::Assign) },
        { "*=",  Token::Punctuator(TokenType::IMul) },
        { "/=",  Token::Punctuator(TokenType::IDiv) },
        { "%=",  Token::Punctuator(TokenType::IMod) },
        { "+=",  Token::Punctuator(TokenType::IPlus) },
        { "-=",  Token::Punctuator(TokenType::IMinus) },
        { "<<=", Token::Punctuator(TokenType::ILShift) },
        { ">>=", Token::Punctuator(TokenType::IRShift) },
        { "&=",  Token::Punctuator(TokenType::IAnd) },
        { "^=",  Token::Punctuator(TokenType::IXor) },
        { "|=",  Token::Punctuator(TokenType::IOr) },
        { ",",   Token::Punctuator(TokenType::Comma) },
        { "#",   Token::Punctuator(TokenType::Hashtag) },
        { "##",  Token::Punctuator(TokenType::HHashtag) },
};

#endif //EPICALYX_PUNCTUATORS_H
