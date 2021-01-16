#ifndef EPICALYX_POSTFIX_EXPRESSION_H
#define EPICALYX_POSTFIX_EXPRESSION_H

#include <memory>
#include <utility>
#include <vector>
#include "../AST.h"

class PostfixExpression : public Expr {
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
            std::unique_ptr<Expr> left,
            std::unique_ptr<Expr> right
            ) : PostfixExpression(PostExprType::ArrayAccess) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    std::unique_ptr<Expr> Left;
    std::unique_ptr<Expr> Right;
};

class FunctionCallExpression : public PostfixExpression {
public:
    explicit FunctionCallExpression(
            std::unique_ptr<Expr> func,
            std::unique_ptr<Expr> args
            ) : PostfixExpression(PostExprType::FunctionCall) {
        this->Func = std::move(func);
        this->Args = std::move(args);
    }

    std::unique_ptr<Expr> Func;
    std::unique_ptr<Expr> Args;
};

class MemberAccessExpression : public PostfixExpression {
public:
    enum class MemberAccessType {
        Direct,   // .
        Pointer,  // ->
    };

    explicit MemberAccessExpression(
            std::unique_ptr<Expr> left,
            std::string& member,
            MemberAccessType access_type
            ) : PostfixExpression(PostExprType::MemberAccess) {
        this->Left = std::move(left);
        this->Member = member;
        this-> AccessType = access_type;
    }

    std::unique_ptr<Expr> Left;
    std::string Member;
    MemberAccessType AccessType;
};


class PostCrementExpression : public PostfixExpression {
public:
    enum class CrementType {
        Increment,
        Decrement,
    };

    explicit PostCrementExpression(
            std::unique_ptr<Expr> left,
            CrementType crement_type
            ) : PostfixExpression(PostExprType::Crement) {
        this->Left = std::move(left);
        this-> CrementType = crement_type;
    }

    std::unique_ptr<Expr> Left;
    CrementType CrementType;
};


class TypeInitializerExpression : public PostfixExpression {
public:

    explicit TypeInitializerExpression(
            std::vector<Expr> args
    ) : PostfixExpression(PostExprType::TypeInitializer) {
        this->Args = std::move(args);
    }

    std::vector<Expr> Args;
};


#endif //EPICALYX_POSTFIX_EXPRESSION_H
