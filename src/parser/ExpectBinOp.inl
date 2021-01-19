#include "expression/binop_expression.h"
#include "utils.h"

template<
        NODE(Expr) (Parser::*SubNode)(),
        enum TokenType... types
> NODE(Expr) Parser::ExpectBinOpExpression() {
    NODE(Expr) node = (this->*SubNode)();
    while (!EndOfStream() && Is(Current()->Type).AnyOf<types...>()) {
        auto current = Current()->Type;
        EatType(current);
        auto right = (this->*SubNode)();
        node = std::make_unique<BinOpExpression>(BinOpExpression::TokenTypeToBinOp(current), node, right);
    }
    return node;
}