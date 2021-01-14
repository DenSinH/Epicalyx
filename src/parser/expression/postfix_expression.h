#ifndef EPICALYX_POSTFIX_EXPRESSION_H
#define EPICALYX_POSTFIX_EXPRESSION_H

#include <memory>
#include <utility>
#include <vector>
#include "../AST.h"
#include "unary_expression.h"

class Expression;
class ArgumentExpressionList;
class AssignmentExpression;


class PostfixExpression : public UnaryExpression {
public:
    enum class PostExprType {
        PrimaryExpression,
        ArrayAccess,
        FunctionCall,
        MemberAccess,
        Crement,  // in/de
        TypeInitializer,
    };

    explicit PostfixExpression(PostExprType type) : UnaryExpression(UnExprType::PostFix) {
        this->Type = type;
    }

    PostExprType Type;
};

class ArrayAccessExpression : public PostfixExpression {
public:
    explicit ArrayAccessExpression(
            std::unique_ptr<PostfixExpression> left,
            std::unique_ptr<Expression> right
            ) : PostfixExpression(PostExprType::ArrayAccess) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    std::unique_ptr<PostfixExpression> Left;
    std::unique_ptr<Expression> Right;
};

class FunctionCallExpression : public PostfixExpression {
public:
    explicit FunctionCallExpression(
            std::unique_ptr<PostfixExpression> func,
            std::unique_ptr<ArgumentExpressionList> args
            ) : PostfixExpression(PostExprType::FunctionCall) {
        this->Func = std::move(func);
        this->Args = std::move(args);
    }

    std::unique_ptr<PostfixExpression> Func;
    std::unique_ptr<ArgumentExpressionList> Args;
};

class MemberAccessExpression : public PostfixExpression {
public:
    enum class MemberAccessType {
        Direct,   // .
        Pointer,  // ->
    };

    explicit MemberAccessExpression(
            std::unique_ptr<PostfixExpression> left,
            std::string& member,
            MemberAccessType access_type
            ) : PostfixExpression(PostExprType::MemberAccess) {
        this->Left = std::move(left);
        this->Member = member;
        this-> AccessType = access_type;
    }

    std::unique_ptr<PostfixExpression> Left;
    std::string Member;
    MemberAccessType AccessType;
};


class CrementExpression : public PostfixExpression {
public:
    enum class CrementType {
        Increment,
        Decrement,
    };

    explicit CrementExpression(
            std::unique_ptr<PostfixExpression> left,
            CrementType crement_type
            ) : PostfixExpression(PostExprType::Crement) {
        this->Left = std::move(left);
        this-> CrementType = crement_type;
    }

    std::unique_ptr<PostfixExpression> Left;
    CrementType CrementType;
};


class TypeInitializerExpression : public PostfixExpression {
public:

    explicit TypeInitializerExpression(
            std::vector<AssignmentExpression> args
    ) : PostfixExpression(PostExprType::TypeInitializer) {
        this->Args = std::move(args);
    }

    std::vector<AssignmentExpression> Args;
};


#endif //EPICALYX_POSTFIX_EXPRESSION_H
