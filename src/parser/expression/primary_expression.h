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
    explicit PrimaryExpressionIdentifier(const std::string& id) :
        PrimaryExpression(PrimExprType::Identifier),
        ID(id) {

    }

    const std::string ID;

    std::list<std::string> Repr() const override {
        return { std::string("Identifier: ") + ID };
    }

    bool IsConstant() const override {
        // todo: We don't know this yet?
        return false;
    }
};

template<typename T>
class PrimaryExpressionConstant : public PrimaryExpression {
public:
    explicit PrimaryExpressionConstant(const T& value) :
        PrimaryExpression(PrimExprType::Constant),
        Value(value) {

    }

    const T Value;

    std::list<std::string> Repr() const override {
        return { std::string("Constant: ") + std::to_string(Value) };
    }

    bool IsConstant() const override {
        return true;
    }
};

class PrimaryStringLiteral : public PrimaryExpression {
public:
    explicit PrimaryStringLiteral(const std::string& value) :
        PrimaryExpression(PrimExprType::StringLiteral),
        Value(value) {
    }

    const std::string Value;

    std::list<std::string> Repr() const override {
        return { std::string("Constant String: ") + Value };
    }

    bool IsConstant() const override {
        return true;
    }
};

#endif //EPICALYX_PRIMARY_EXPRESSION_H
