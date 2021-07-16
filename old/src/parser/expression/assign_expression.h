#ifndef EPICALYX_ASSIGN_EXPRESSION_H
#define EPICALYX_ASSIGN_EXPRESSION_H

#include "AST.h"
#include "../../tokenizer/tokens.h"

#include <map>
#include <stdexcept>

class AssignmentExpression : public ExprNode {
public:
    enum class AssignOp {
        Eq,
        MulEq,
        DivEq,
        ModEq,
        AddEq,
        SubEq,
        LShiftEq,
        RShiftEq,
        AndEq,
        XorEq,
        OrEq,
    };

    AssignmentExpression(
            const TOKEN& tok,
            NODE(ExprNode)&& left,
            AssignOp op,
            NODE(ExprNode)&& right) : ExprNode(tok) {
        this->Left = std::move(left);
        this->Op = op;
        this->Right = std::move(right);
    }

    NODE(ExprNode) Left;
    AssignOp Op;
    NODE(ExprNode) Right;  // can also be a CondExpr

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "AssignExpr:" };
        NestedRepr(repr, Left);
        repr.emplace_back(Operation());
        NestedRepr(repr, Right);
        return repr;
    }

    static AssignOp TokenTypeToAssignOp(enum TokenType type) {
        if (TokenMap.contains(type)) {
            return TokenMap.at(type);
        }
        throw std::runtime_error("Invalid assignment token type: " + Token::TypeString(type));
    }

private:
    static const std::map<enum TokenType, AssignOp> TokenMap;
    static const std::map<AssignOp, std::string> OpString;
    
    std::string Operation() const {
        return OpString.at(Op);
    }
};

#endif //EPICALYX_ASSIGN_EXPRESSION_H
