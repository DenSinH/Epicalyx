#ifndef EPICALYX_PARSER_H
#define EPICALYX_PARSER_H

#include "../tokenizer/tokenizer.h"
#include "../tokenizer/tokens.h"
#include "AST.h"
#include "log.h"

class Parser {
public:
    explicit Parser(std::vector<std::shared_ptr<Token>>& tokens) {
        this->Tokens = tokens;
    }

    explicit Parser(Tokenizer& tokenizer) {
        this->Tokens = tokenizer.Tokens;
    }

    explicit Parser(Tokenizer* tokenizer) {
        this->Tokens = tokenizer->Tokens;
    }

    std::shared_ptr<Node> Parse();

    std::shared_ptr<Token> Current() {
        if (Index < Tokens.size()) {
            return Tokens[Index];
        }
        log_fatal("Unexpected end of program");
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

    std::shared_ptr<Token> Next() {
        return Tokens[Index + 1];
    }

    std::shared_ptr<Token> ExpectType(enum TokenType type) {
        auto current = Current();
        if (current->Type != type) {
            log_fatal("Unexpected token");
        }
        return current;
    }

    std::shared_ptr<Token> EatType(enum TokenType type) {
        auto eaten = ExpectType(type);
        Advance();
        return eaten;
    }
    
private:
    friend int main();
    std::vector<std::shared_ptr<Token>> Tokens;
    unsigned long long Index = 0;

    std::shared_ptr<Expr> ExpectPrimaryExpression();
    std::shared_ptr<Expr> ExpectPostfixExpression();
};

#endif //EPICALYX_PARSER_H
