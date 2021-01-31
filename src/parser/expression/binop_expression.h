#ifndef EPICALYX_BINOP_EXPRESSION_H
#define EPICALYX_BINOP_EXPRESSION_H

#include <stdexcept>
#include "log.h"
#include "../AST.h"

#include <map>

class BinOpExpression : public ExprNode {
public:
    enum class BinOp {
        Mul,
        Div,
        Mod,
        Add,
        Sub,
        LShift,
        RShift,
        Leq,
        Geq,
        Lt,
        Gt,
        Eq,
        Ne,
        BinAnd,
        BinXor,
        BinOr,
        LogicAnd,
        LogicOr,
    };

    BinOpExpression(const TOKEN& tok, BinOp op, NODE(ExprNode)&& left, NODE(ExprNode)&& right) :
        ExprNode(tok),
        Op(op) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    const BinOp Op;
    NODE(ExprNode) Left;
    NODE(ExprNode) Right;

    static BinOp TokenTypeToBinOp(const enum TokenType type) {
        if (TokenMap.contains(type)) {
            return TokenMap.at(type);
        }
        throw std::runtime_error("Invalid BinOp token: " + Token::TypeString(type));
    }

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "BinOp: " + Operation() };
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }

    bool IsConstant() const override {
        return Left->IsConstant() && Right->IsConstant();
    }

private:
    static const std::map<BinOp, std::string> OpString;
    static const std::map<enum TokenType, BinOp> TokenMap;

    std::string Operation() const {
        return OpString.at(Op);
    }
};

#endif //EPICALYX_BINOP_EXPRESSION_H
