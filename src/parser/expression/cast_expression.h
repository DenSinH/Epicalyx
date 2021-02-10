#ifndef EPICALYX_CAST_EXPRESSION_H
#define EPICALYX_CAST_EXPRESSION_H

#include "AST.h"

class TypeName;

// CondExpr can also just be a CastExpression
class CastExpression : public ExprNode {
public:
    CastExpression(const TOKEN& tok, NODE(TypeName)&& type, NODE(ExprNode)&& right);

    NODE(TypeName) Type;
    NODE(ExprNode) Right;

    std::list<std::string> Repr() const override;  // requires info on TypeName struct

    bool IsConstant(const ParserState& state) const override {
        return Right->IsConstant(state);
    }
};

#endif //EPICALYX_CAST_EXPRESSION_H
