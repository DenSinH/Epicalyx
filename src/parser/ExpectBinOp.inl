#include "expression/binop_expression.h"
#include "utils.h"

template<
        NODE(ExprNode) (Parser::*SubNode)(),
        enum TokenType... types
> NODE(ExprNode) Parser::ExpectBinOpExpression() {
    NODE(ExprNode) node = (this->*SubNode)();
    while (!EndOfStream() && Is(Current()->Type).AnyOf<types...>()) {
        auto current = Current()->Type;
        EatType(current);
        auto right = (this->*SubNode)();
        node = std::make_unique<BinOpExpression>(BinOpExpression::TokenTypeToBinOp(current), node, right);
    }
    return node;
}