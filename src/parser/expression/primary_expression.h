#ifndef EPICALYX_PRIMARY_EXPRESSION_H
#define EPICALYX_PRIMARY_EXPRESSION_H

#include "../AST.h"

class PrimaryExpression : public Node {
public:
    enum class PEType {
        Identifier,
        Constant,
        StringLiteral,
        Expression,
        // todo: generic selection
    };

    explicit PrimaryExpression(PrimaryExpression::PEType type) {
        this->Type = type;
    }

    PrimaryExpression::PEType Type;
};

class PrimaryExpressionIdentifier : public PrimaryExpression {
public:
    explicit PrimaryExpressionIdentifier(std::string& id) : PrimaryExpression(PEType::Identifier) {
        this->ID = id;
    }

    std::string ID;
};

class PrimaryExpressionIdentifier : public PrimaryExpression {
public:
    explicit PrimaryExpressionIdentifier(std::string& id) : PrimaryExpression(PEType::Identifier) {
        this->ID = id;
    }

    std::string ID;
};

#endif //EPICALYX_PRIMARY_EXPRESSION_H
