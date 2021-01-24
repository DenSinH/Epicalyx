#ifndef EPICALYX_PRIMARY_EXPRESSION_H
#define EPICALYX_PRIMARY_EXPRESSION_H

#include "../AST.h"

class PrimaryExpression : public ExprNode {
public:
    enum class PrimExprType {
        Identifier,
        Constant,
        StringLiteral,
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

    std::list<std::string> Repr() override {
        return { std::string("Identifier: ") + ID };
    }

    bool IsConstant() override {
        // todo: We don't know this yet?
        return false;
    }
};

template<typename T>
class PrimaryExpressionConstant : public PrimaryExpression {
public:
    explicit PrimaryExpressionConstant(T& value) : PrimaryExpression(PrimExprType::Constant) {
        this->Value = value;
    }

    T Value;

    std::list<std::string> Repr() override {
        return { std::string("Constant: ") + std::to_string(Value) };
    }

    bool IsConstant() override {
        return true;
    }
};

class PrimaryStringLiteral : public PrimaryExpression {
public:
    explicit PrimaryStringLiteral(std::string& value) : PrimaryExpression(PrimExprType::StringLiteral) {
        this->Value = value;
    }

    std::string Value;

    std::list<std::string> Repr() override {
        return { std::string("Constant String: ") + Value };
    }

    bool IsConstant() override {
        return true;
    }
};

#endif //EPICALYX_PRIMARY_EXPRESSION_H
