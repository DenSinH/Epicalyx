#ifndef EPICALYX_PRIMARY_EXPRESSION_H
#define EPICALYX_PRIMARY_EXPRESSION_H

#include "../AST.h"

class PrimaryExpression : public Expr {
public:
    enum class PrimExprType {
        Identifier,
        Constant,
        StringLiteral,
        Expression,
        // todo: generic selection
    };

    explicit PrimaryExpression(PrimaryExpression::PrimExprType type) {
        this->Type = type;
    }

    PrimaryExpression::PrimExprType Type;
};

class PrimaryExpressionIdentifier : public PrimaryExpression {
public:
    explicit PrimaryExpressionIdentifier(std::string& id) : PrimaryExpression(PrimExprType::Identifier) {
        this->ID = id;
    }

    std::string ID;

    std::vector<std::string> Repr() override {
        return { std::string("Identifier: ") + ID };
    }
};

template<typename T>
class PrimaryExpressionConstant : public PrimaryExpression {
public:
    explicit PrimaryExpressionConstant(T& value) : PrimaryExpression(PrimExprType::Constant) {
        this->Value = value;
    }

    T Value;

    std::vector<std::string> Repr() override {
        return { std::string("Constant: ") + std::to_string(Value) };
    }
};

class PrimaryStringLiteral : public PrimaryExpression {
public:
    explicit PrimaryStringLiteral(std::string& value) : PrimaryExpression(PrimExprType::StringLiteral) {
        this->Value = value;
    }

    std::string Value;

    std::vector<std::string> Repr() override {
        return { std::string("Constant String: ") + Value };
    }
};

#endif //EPICALYX_PRIMARY_EXPRESSION_H
