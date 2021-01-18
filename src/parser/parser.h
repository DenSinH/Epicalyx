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

    std::unique_ptr<Node> Parse();

    std::shared_ptr<Token> Current() {
        return Tokens[Index];
    }

    void Advance() {
        Index++;
    }

    bool HasNext() {
        return Index < Tokens.size() - 1;
    }

    std::shared_ptr<Token> Next() {
        return Tokens[Index + 1];
    }

    void ExpectType(enum TokenType type) {
        if (Current()->Type != type) {
            log_fatal("Unexpected token");
        }
    }

    void EatType(enum TokenType type) {
        ExpectType(type);
        Advance();
    }
    
private:
    friend int main();
    std::vector<std::shared_ptr<Token>> Tokens;
    unsigned long long Index = 0;

    std::unique_ptr<Node> ExpectPrimaryExpression();
};

#endif //EPICALYX_PARSER_H
