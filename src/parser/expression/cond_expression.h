#ifndef EPICALYX_COND_EXPRESSION_H
#define EPICALYX_COND_EXPRESSION_H

#include "../AST.h"

class CondExpression : public ExprNode {
public:
    CondExpression(
            const TOKEN& tok,
            NODE(ExprNode)&& left,
            NODE(ExprNode)&& t,
            NODE(ExprNode)&& f) :
                ExprNode(tok),
                Left(std::move(left)),
                True(std::move(t)),
                False(std::move(f)) {

    }

    const NODE(ExprNode) Left;
    const NODE(ExprNode) True;
    const NODE(ExprNode) False;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "CondExpr: Cond:" };
        NestedRepr(repr, Left);
        repr.emplace_back("True:");
        NestedRepr(repr, True);
        repr.emplace_back("False:");
        NestedRepr(repr, False);
        return repr;
    }

    bool IsConstant(const ParserState& state) const override {
        if (!Left->IsConstant(state)) {
            return false;
        }

        auto left_value = Left->ConstEval(state);
        if (left_value->GetBoolValue()) {
            return True->IsConstant(state);
        }
        else {
            return False->IsConstant(state);
        }
    }
};


#endif //EPICALYX_COND_EXPRESSION_H
