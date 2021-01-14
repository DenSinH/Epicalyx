#ifndef EPICALYX_PUNCTUATORS_H
#define EPICALYX_PUNCTUATORS_H

#include "tokens.h"

#include <map>
#include <string>


static const std::map<const std::string, const Token> Punctuators = {
        { "[",   Punctuator(TokenType::LBracket, PunctuatorFlags::None) },
        { "]",   Punctuator(TokenType::RBracket, PunctuatorFlags::None) },
        { "(",   Punctuator(TokenType::LParen, PunctuatorFlags::None) },
        { ")",   Punctuator(TokenType::RParen, PunctuatorFlags::None) },
        { "{",   Punctuator(TokenType::LBrace, PunctuatorFlags::None) },
        { "}",   Punctuator(TokenType::RBrace, PunctuatorFlags::None) },
        { ".",   Punctuator(TokenType::Dot, PunctuatorFlags::MemberAccess) },
        { "->",  Punctuator(TokenType::Arrow, PunctuatorFlags::MemberAccess) },
        { "++",  Punctuator(TokenType::Incr, PunctuatorFlags::PostFix) },
        { "--",  Punctuator(TokenType::Decr, PunctuatorFlags::PostFix) },
        { "&",   Punctuator(TokenType::Ampersand, PunctuatorFlags::BinOp | PunctuatorFlags::UnOp) },
        { "*",   Punctuator(TokenType::Asterisk, PunctuatorFlags::BinOp | PunctuatorFlags::UnOp) },
        { "+",   Punctuator(TokenType::Plus, PunctuatorFlags::BinOp | PunctuatorFlags::UnOp) },
        { "-",   Punctuator(TokenType::Minus, PunctuatorFlags::BinOp | PunctuatorFlags::UnOp) },
        { "~",   Punctuator(TokenType::Tilde, PunctuatorFlags::UnOp) },
        { "!",   Punctuator(TokenType::Exclamation, PunctuatorFlags::UnOp) },
        { "/",   Punctuator(TokenType::Div, PunctuatorFlags::BinOp) },
        { "%",   Punctuator(TokenType::Mod, PunctuatorFlags::BinOp) },
        { "<<",  Punctuator(TokenType::LShift, PunctuatorFlags::BinOp) },
        { ">>",  Punctuator(TokenType::RShift, PunctuatorFlags::BinOp) },
        { "<",   Punctuator(TokenType::Less, PunctuatorFlags::Relational) },
        { ">",   Punctuator(TokenType::Greater, PunctuatorFlags::Relational) },
        { "<=",  Punctuator(TokenType::LessEqual, PunctuatorFlags::Relational) },
        { ">=",  Punctuator(TokenType::GreaterEqual, PunctuatorFlags::Relational) },
        { "==",  Punctuator(TokenType::Equal, PunctuatorFlags::Relational) },
        { "!=",  Punctuator(TokenType::NotEqual, PunctuatorFlags::Relational) },
        { "^",   Punctuator(TokenType::BinXor, PunctuatorFlags::BinOp) },
        { "|",   Punctuator(TokenType::BinOr, PunctuatorFlags::BinOp) },
        { "&&",  Punctuator(TokenType::LogicalAnd, PunctuatorFlags::BinOp) },
        { "||",  Punctuator(TokenType::LogicalOr, PunctuatorFlags::BinOp) },
        { "?",   Punctuator(TokenType::Question, PunctuatorFlags::None) },
        { ":",   Punctuator(TokenType::Colon, PunctuatorFlags::None) },
        { ";",   Punctuator(TokenType::SemiColon, PunctuatorFlags::None) },
        { "...", Punctuator(TokenType::Ellipsis, PunctuatorFlags::None) },
        { "=",   Punctuator(TokenType::Assign, PunctuatorFlags::Assignment) },
        { "*=",  Punctuator(TokenType::IMul, PunctuatorFlags::Assignment) },
        { "/=",  Punctuator(TokenType::IDiv, PunctuatorFlags::Assignment) },
        { "%=",  Punctuator(TokenType::IMod, PunctuatorFlags::Assignment) },
        { "+=",  Punctuator(TokenType::IPlus, PunctuatorFlags::Assignment) },
        { "-=",  Punctuator(TokenType::IMinus, PunctuatorFlags::Assignment) },
        { "<<=", Punctuator(TokenType::ILShift, PunctuatorFlags::Assignment) },
        { ">>=", Punctuator(TokenType::IRShift, PunctuatorFlags::Assignment) },
        { "&=",  Punctuator(TokenType::IAnd, PunctuatorFlags::Assignment) },
        { "^=",  Punctuator(TokenType::IXor, PunctuatorFlags::Assignment) },
        { "|=",  Punctuator(TokenType::IOr, PunctuatorFlags::Assignment) },
        { ",",   Punctuator(TokenType::Comma, PunctuatorFlags::None) },
        { "#",   Punctuator(TokenType::Hashtag, PunctuatorFlags::None) },
        { "##",  Punctuator(TokenType::HHashtag, PunctuatorFlags::None) },
};

#endif //EPICALYX_PUNCTUATORS_H
