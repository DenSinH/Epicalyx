#include "expression/binop_expression.h"
#include "utils.h"

template<
        NODE(ExprNode) (Parser::*SubNode)(),
        enum TokenType... types
> NODE(ExprNode) Parser::ExpectBinOpExpression() {
    NODE(ExprNode) node = (this->*SubNode)();
    while (!EndOfStream() && Is(Current()->Type).AnyOf<types...>()) {
        auto current = Current();
        Advance();
        auto right = (this->*SubNode)();
        node = MAKE_NODE(BinOpExpression)(current, BinOpExpression::TokenTypeToBinOp(current->Type), node, right);
    }
    return node;
}