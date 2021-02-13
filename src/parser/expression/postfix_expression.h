#ifndef EPICALYX_POSTFIX_EXPRESSION_H
#define EPICALYX_POSTFIX_EXPRESSION_H

#include "AST.h"

class TypeName;
class Initializer;

class PostfixExpression : public ExprNode {
public:
    enum class PostExprType {
        PrimaryExpression,
        ArrayAccess,
        FunctionCall,
        MemberAccess,
        Crement,  // in/de
        TypeInitializer,
    };

    explicit PostfixExpression(const TOKEN& tok, PostExprType type) : ExprNode(tok) {
        this->Type = type;
    }

    PostExprType Type;
};

class ArrayAccessExpression : public PostfixExpression {
public:
    explicit ArrayAccessExpression(
            const TOKEN& tok,
            NODE(ExprNode)&& left,
            NODE(ExprNode)&& right
            ) :
                PostfixExpression(tok, PostExprType::ArrayAccess),
                Left(std::move(left)),
                Right(std::move(right)) {

    }

    const NODE(ExprNode) Left;
    const NODE(ExprNode) Right;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "ArrayAccessExpr:" };
        NestedRepr(repr, Left);
        repr.emplace_back("Element");
        NestedRepr(repr, Right);
        return repr;
    }

    bool IsConstant(const ParserState& state) const override { return Left->IsConstant(state) && Right->IsConstant(state); }
    CTYPE SemanticAnalysis(const ParserState& state) const override;
};

class FunctionCallExpression : public PostfixExpression {
public:
    explicit FunctionCallExpression(
            const TOKEN& tok,
            NODE(ExprNode)&& func
            ) :
                PostfixExpression(tok, PostExprType::FunctionCall),
                Func(std::move(func)) {

    }

    void AddArgument(NODE(ExprNode)&& arg) {
        Args.push_back(std::move(arg));
    }

    const NODE(ExprNode) Func;
    std::vector<NODE(ExprNode)> Args;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "FunctionCallExpr:" };
        NestedRepr(repr, Func);
        repr.emplace_back("Args");
        NestedRepr(repr, Args);
        return repr;
    }
    CTYPE SemanticAnalysis(const ParserState& state) const override;
};

class MemberAccessExpression : public PostfixExpression {
public:
    enum class MemberAccessType {
        Direct,   // .
        Pointer,  // ->
    };

    explicit MemberAccessExpression(
            const TOKEN& tok,
            NODE(ExprNode)&& left,
            const std::string& member,
            MemberAccessType access_type
            ) :
                PostfixExpression(tok, PostExprType::MemberAccess),
                Left(std::move(left)),
                Member(member),
                AccessType(access_type) {

    }

    const NODE(ExprNode) Left;
    const std::string Member;
    const MemberAccessType AccessType;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { std::string("MemberAccessExpr:") + (AccessType == MemberAccessType::Direct ? "." : "->") + Member};
        NestedRepr(repr, Left);
        return repr;
    }

    bool IsConstant(const ParserState& state) const override { return Left->IsConstant(state); }
    CTYPE SemanticAnalysis(const ParserState& state) const override;
};


class PostCrementExpression : public PostfixExpression {
public:
    enum class CrementType {
        Increment,
        Decrement,
    };

    explicit PostCrementExpression(
            const TOKEN& tok,
            NODE(ExprNode)&& left,
            CrementType type
            ) :
                PostfixExpression(tok, PostExprType::Crement),
                Left(std::move(left)),
                Type(type) {

    }

    const NODE(ExprNode) Left;
    const CrementType Type;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { std::string("PostCrementExpression:") + (Type == CrementType::Increment ? "++" : "--") };
        NestedRepr(repr, Left);
        return repr;
    }
    CTYPE SemanticAnalysis(const ParserState& state) const override;
};


class InitializerList;

class TypeInitializerExpression : public PostfixExpression {
public:

    explicit TypeInitializerExpression(
            const TOKEN& tok,
            NODE(TypeName)&& type,
            NODE(InitializerList)&& initializers
    );

    const NODE(TypeName) Type;
    const NODE(InitializerList) Initializers = {};

    std::list<std::string> Repr() const override;  // requires info on InitializerList struct
    CTYPE SemanticAnalysis(const ParserState& state) const override;
};


#endif //EPICALYX_POSTFIX_EXPRESSION_H
