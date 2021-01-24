#ifndef EPICALYX_ASSIGN_EXPRESSION_H
#define EPICALYX_ASSIGN_EXPRESSION_H

#include "../AST.h"
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
            NODE(ExprNode)& left,
            AssignOp op,
            NODE(ExprNode)& right) {
        this->Left = std::move(left);
        this->Op = op;
        this->Right = std::move(right);
    }

    NODE(ExprNode) Left;
    AssignOp Op;
    NODE(ExprNode) Right;  // can also be a CondExpr

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "AssignExpr:" };
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        repr.emplace_back(Operation());
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
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
    
    std::string Operation() {
        return OpString.at(Op);
    }
};

#endif //EPICALYX_ASSIGN_EXPRESSION_H
