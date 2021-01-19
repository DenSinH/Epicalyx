#include "expression/binop_expression.h"

template<
        NODE(Expr) (Parser::*SubNode)(),
        enum TokenType... types
> NODE(Expr) Parser::ExpectBinOpExpression() {
    NODE(Expr) node = (this->*SubNode)();
    while (!EndOfStream() && IsAny<enum TokenType, types...>(Current()->Type)) {
        auto current = Current()->Type;
        EatType(current);
        auto right = (this->*SubNode)();
        node = std::make_unique<BinOpExpression>(BinOpExpression::TokenTypeToBinOp(current), node, right);
    }
    return node;
}