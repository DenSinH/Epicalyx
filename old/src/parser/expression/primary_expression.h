#ifndef EPICALYX_PRIMARY_EXPRESSION_H
#define EPICALYX_PRIMARY_EXPRESSION_H

#include "AST.h"
#include "types/Types.h"

class PrimaryExpression : public ExprNode {
public:
    enum class PrimExprType {
        Identifier,
        Constant,
        StringLiteral,
        // todo: generic selection
    };

    explicit PrimaryExpression(const TOKEN& tok, PrimaryExpression::PrimExprType type) : ExprNode(tok) {
        this->Type = type;
    }

    PrimaryExpression::PrimExprType Type;
};

class PrimaryExpressionIdentifier : public PrimaryExpression {
public:
    explicit PrimaryExpressionIdentifier(const TOKEN& tok, std::string name) :
            PrimaryExpression(tok, PrimExprType::Identifier),
            Name(std::move(name)) {

    }

    const std::string Name;

    std::list<std::string> Repr() const override {
        return {std::string("Identifier: ") + Name };
    }

    bool IsConstant(const ParserState& state) const override; // requires knowledge about the ParserState struct
    CTYPE SemanticAnalysis(const ParserState& state) const override;
};

template<typename T>
class PrimaryExpressionConstant : public PrimaryExpression {
public:
    explicit PrimaryExpressionConstant(const TOKEN& tok, const T& value) :
        PrimaryExpression(tok, PrimExprType::Constant),
        Value(value) {

    }

    explicit PrimaryExpressionConstant(const NumericalConstantToken<T>& tok) :
            PrimaryExpression(tok, PrimExprType::Constant),
            Value(tok.Value) {

    }

    const T Value;

    std::list<std::string> Repr() const override {
        return { std::string("Constant: ") + std::to_string(Value) };
    }

    bool IsConstant(const ParserState&) const override { return true; }

    CTYPE SemanticAnalysis(const ParserState&) const override {
        return MAKE_TYPE(ValueType<T>)(Value, CType::LValueNess::None);
    }
};

class PrimaryStringLiteral : public PrimaryExpression {
public:
    explicit PrimaryStringLiteral(const TOKEN& tok, const std::string& value) :
        PrimaryExpression(tok, PrimExprType::StringLiteral),
        Value(value) {
    }

    const std::string Value;

    std::list<std::string> Repr() const override {
        return { std::string("Constant String: ") + Value };
    }

    bool IsConstant(const ParserState&) const override { return true; }
    CTYPE SemanticAnalysis(const ParserState& state) const override;
};

#endif //EPICALYX_PRIMARY_EXPRESSION_H
