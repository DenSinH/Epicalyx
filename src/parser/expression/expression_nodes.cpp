#include "expression_nodes.h"
#include "declaration/typename.h"
#include "declaration/initializer.h"
#include "parser_state.h"
#include "types/types.h"


CastExpression::CastExpression(const TOKEN& tok, NODE(TypeName)&& type, NODE(ExprNode)&& right)  :
        ExprNode(tok),
        Type(std::move(type)),
        Right(std::move(right)) {

}

std::list<std::string> CastExpression::Repr() const  {
    std::list<std::string> repr = { "CastExpr:" };
    NestedRepr(repr, Type);

    repr.emplace_back("Value:");
    NestedRepr(repr, Right);
    return repr;
}

TypeInitializerExpression::TypeInitializerExpression(
        const TOKEN& tok,
        NODE(TypeName)&& type,
        NODE(InitializerList)&& initializers
    ) :
        PostfixExpression(tok,  PostExprType::TypeInitializer),
        Type(std::move(type)),
        Initializers(std::move(initializers)) {

}

std::list<std::string> TypeInitializerExpression::Repr() const {
    std::list<std::string> repr = { "TypeInitializerExpr:" };

    NestedRepr(repr, Type);
    NestedRepr(repr, Initializers);
    return repr;
}

SizeOfTypeExpression::SizeOfTypeExpression(const TOKEN& tok, NODE(TypeName)&& right) :
        UnaryExpression(tok, UnExprType::SizeOf),
        Right(std::move(right)) {

}

std::list<std::string> SizeOfTypeExpression::Repr() const {
    std::list<std::string> repr = { "SizeOfTypeExpression: " };
    NestedRepr(repr, Right);
    return repr;
}

AlignOfExpression::AlignOfExpression(const TOKEN& tok, NODE(TypeName)& right) :
        UnaryExpression(tok, UnExprType::AlignOf),
        Right(std::move(right)) {

}

std::list<std::string> AlignOfExpression::Repr() const {
    std::list<std::string> repr = { "AlignOf: " };
    NestedRepr(repr, Right);
    return repr;
}

bool PrimaryExpressionIdentifier::IsConstant(const ParserState& state) const {
    return state.IsConstant(Name);
}

CTYPE PrimaryExpressionIdentifier::GetType(const ParserState& state) const {
    return state.GetType(Name);
}

CTYPE PrimaryStringLiteral::GetType(const ParserState&) const {
    // not assignable (spec says it is undefined)
    TYPE(ValueType<i8>) Char;
    if (Value.empty()) {
        Char = MAKE_TYPE(ValueType<i8>)(0, CType::LValueNess::None);
    }
    else {
        Char = MAKE_TYPE(ValueType<i8>)(Value[0], CType::LValueNess::None);
    }

    // null terminator
    // todo: store string literals
    return MAKE_TYPE(ArrayType)(Char, Value.size() + 1);
}