#ifndef EPICALYX_PARSER_H
#define EPICALYX_PARSER_H

#include "../tokenizer/tokenizer.h"
#include "../tokenizer/tokens.h"
#include "AST.h"
#include "log.h"

#include <set>
#include <stdexcept>

class StaticAssertDecl;
class Initializer;

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

    NODE(Node) Parse() { return nullptr; };

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

    bool HasAfter(size_t offset) {
        return (Index + offset) < (Tokens.size() - 1);
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
            throw std::runtime_error("Unexpected token, got " + current->Repr() + ", expected: " + Token::TypeString(current->Type));
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

    std::set<std::string> TypedefNames = {};
    bool IsTypeName(size_t after);

    NODE(Expr) ExpectPrimaryExpression();
    NODE(Expr) ExpectPostfixExpression();
    NODE(Expr) ExpectArgumentListExpression();
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
    NODE(Expr) ExpectConditionalExpression();
    NODE(Expr) ExpectConstantExpression() { return ExpectConditionalExpression(); }
    NODE(Expr) ExpectAssignmentExpression();
    NODE(Expr) ExpectExpression();

    NODE(StaticAssertDecl) ExpectStaticAssert();
    NODE(Initializer) ExpectInitializer();

    template<
            NODE(Expr) (Parser::*SubNode)(),
            enum TokenType... type
    > NODE(Expr) ExpectBinOpExpression();
};

#include "ExpectBinOp.inl"

#endif //EPICALYX_PARSER_H
