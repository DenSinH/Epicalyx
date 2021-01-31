#ifndef EPICALYX_POSTFIX_EXPRESSION_H
#define EPICALYX_POSTFIX_EXPRESSION_H

#include "../AST.h"
#include "../declaration/typename.h"
#include "../declaration/initializer.h"

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
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        repr.emplace_back("Element");
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }

    bool IsConstant() const override {
        return Left->IsConstant() && Right->IsConstant();
    }
};

class FunctionCallExpression : public PostfixExpression {
public:
    explicit FunctionCallExpression(
            const TOKEN& tok,
            NODE(ExprNode)&& func
            ) :
                PostfixExpression(tok, PostExprType::FunctionCall),
                Func(std::move(func)),
                Args(nullptr) {

    }

    explicit FunctionCallExpression(
            const TOKEN& tok,
            NODE(ExprNode)&& func,
            NODE(ExprNode)&& args
            ) :
                PostfixExpression(tok, PostExprType::FunctionCall),
                Func(std::move(func)),
                Args(std::move(args)) {

    }

    const NODE(ExprNode) Func;
    const NODE(ExprNode) Args;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "FunctionCallExpr:" };
        for (auto& s : Func->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        repr.emplace_back("Args");
        for (auto& s : Args->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
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
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }

    bool IsConstant() const override {
        return Left->IsConstant();
    }
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
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};


class TypeInitializerExpression : public PostfixExpression {
public:

    explicit TypeInitializerExpression(
            const TOKEN& tok,
            NODE(TypeName)&& type,
            NODE(InitializerList)&& initializers
    ) :
            PostfixExpression(tok,  PostExprType::TypeInitializer),
            Type(std::move(type)),
            Initializers(std::move(initializers)) {

    }

    const NODE(TypeName) Type;
    const NODE(InitializerList) Initializers = {};

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "TypeInitializerExpr:" };

        for (auto& s : Type->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }

        for (auto& s : Initializers->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};


#endif //EPICALYX_POSTFIX_EXPRESSION_H
