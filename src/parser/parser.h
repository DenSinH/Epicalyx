#ifndef EPICALYX_PARSER_H
#define EPICALYX_PARSER_H

#include "../tokenizer/tokenizer.h"
#include "../tokenizer/tokens.h"
#include "AST.h"
#include "log.h"

#include <stdexcept>

#define NODE(_type) std::unique_ptr<_type>

class Parser {
public:
    explicit Parser(std::vector<TOKEN>& tokens) {
        this->Tokens = tokens;
    }

    explicit Parser(Tokenizer& tokenizer) {
        this->Tokens = tokenizer.Tokens;
    }

    explicit Parser(Tokenizer* tokenizer) {
        this->Tokens = tokenizer->Tokens;
    }

    NODE(Node) Parse();

    TOKEN Current() {
        if (Index < Tokens.size()) {
            return Tokens[Index];
        }
        throw std::runtime_error("Unexpected end of program");
    }

    void Advance() {
        Index++;
    }

    bool EndOfStream() {
        return Index >= Tokens.size();
    }

    bool HasNext() {
        return Index < (Tokens.size() - 1);
    }

    TOKEN Next() {
        return Tokens[Index + 1];
    }

    TOKEN LookAhead(int amount) {
        if (Index + amount < Tokens.size()) {
            return Tokens[Index + amount];
        }
        return nullptr;
    }

    TOKEN ExpectType(enum TokenType type) {
        auto current = Current();
        if (current->Type != type) {
            throw std::exception("Unexpected token");
        }
        return current;
    }

    TOKEN EatType(enum TokenType type) {
        auto eaten = ExpectType(type);
        Advance();
        return eaten;
    }
    
private:
    friend int main();
    std::vector<TOKEN> Tokens;
    unsigned long long Index = 0;

    NODE(Expr) ExpectPrimaryExpression();
    NODE(Expr) ExpectPostfixExpression();
    NODE(Expr) ExpectUnaryExpression();
    NODE(Expr) ExpectCastExpression();
    NODE(Expr) ExpectMultExpression();
    NODE(Expr) ExpectAddExpression();
    NODE(Expr) ExpectShiftExpression();
    NODE(Expr) ExpectRelationalExpression();
    NODE(Expr) ExpectEqualityExpression();
    NODE(Expr) ExpectBinAndExpression();
    NODE(Expr) ExpectBinXorExpression();
    NODE(Expr) ExpectBinOrExpression();
    NODE(Expr) ExpectLogicAndExpression();
    NODE(Expr) ExpectLogicOrExpression();

    template<
            NODE(Expr) (Parser::*SubNode)(),
            enum TokenType... type
    > NODE(Expr) ExpectBinOpExpression();

    template<typename T> static bool IsAny(T value) {
        return false;
    }

    template<typename T, T Value, T... Values> static bool IsAny(T value) {
        return (value == Value) || IsAny<T, Values...>(value);
    }
};

#include "ExpectBinOp.inl"

#endif //EPICALYX_PARSER_H
