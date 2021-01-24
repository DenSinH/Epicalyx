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

    explicit PostfixExpression(PostExprType type) {
        this->Type = type;
    }

    PostExprType Type;
};

class ArrayAccessExpression : public PostfixExpression {
public:
    explicit ArrayAccessExpression(
            NODE(ExprNode)& left,
            NODE(ExprNode)& right
            ) : PostfixExpression(PostExprType::ArrayAccess) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    NODE(ExprNode) Left;
    NODE(ExprNode) Right;

    std::list<std::string> Repr() override {
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

    bool IsConstant() override {
        return Left->IsConstant() && Right->IsConstant();
    }
};

class FunctionCallExpression : public PostfixExpression {
public:
    explicit FunctionCallExpression(
            NODE(ExprNode)& func
            ) : PostfixExpression(PostExprType::FunctionCall) {
        this->Func = std::move(func);
        this->Args = nullptr;
    }

    explicit FunctionCallExpression(
            NODE(ExprNode)& func,
            NODE(ExprNode)& args
            ) : PostfixExpression(PostExprType::FunctionCall) {
        this->Func = std::move(func);
        this->Args = std::move(args);
    }

    NODE(ExprNode) Func;
    NODE(ExprNode) Args;

    std::list<std::string> Repr() override {
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
            NODE(ExprNode)& left,
            const std::string& member,
            MemberAccessType access_type
            ) :
            PostfixExpression(PostExprType::MemberAccess),
            Left(std::move(left)),
            Member(member),
            AccessType(access_type) {

    }

    const NODE(ExprNode) Left;
    const std::string Member;
    const MemberAccessType AccessType;

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { std::string("MemberAccessExpr:") + (AccessType == MemberAccessType::Direct ? "." : "->") + Member};
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }

    bool IsConstant() override {
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
            NODE(ExprNode)& left,
            CrementType type
            ) : PostfixExpression(PostExprType::Crement) {
        this->Left = std::move(left);
        this-> Type = type;
    }

    NODE(ExprNode) Left;
    CrementType Type;

    std::list<std::string> Repr() override {
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
            NODE(TypeName)& type,
            NODE(InitializerList)& initializers
    ) : PostfixExpression(PostExprType::TypeInitializer) {
        Type = std::move(type);
        Initializers = std::move(initializers);
    }

    NODE(TypeName) Type;
    NODE(InitializerList) Initializers = {};

    std::list<std::string> Repr() override {
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
