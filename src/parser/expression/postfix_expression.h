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
            NODE(Expr)& left,
            NODE(Expr)& right
            ) : PostfixExpression(PostExprType::ArrayAccess) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    NODE(Expr) Left;
    NODE(Expr) Right;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "ArrayAccessExpr:" };
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        repr.emplace_back("Element");
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class FunctionCallExpression : public PostfixExpression {
public:
    explicit FunctionCallExpression(
            NODE(Expr)& func,
            NODE(Expr)& args
            ) : PostfixExpression(PostExprType::FunctionCall) {
        this->Func = std::move(func);
        this->Args = std::move(args);
    }

    NODE(Expr) Func;
    NODE(Expr) Args;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "FunctionCallExpr:" };
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
            NODE(Expr)& left,
            std::string& member,
            MemberAccessType access_type
            ) : PostfixExpression(PostExprType::MemberAccess) {
        this->Left = std::move(left);
        this->Member = member;
        this-> AccessType = access_type;
    }

    NODE(Expr) Left;
    std::string Member;
    MemberAccessType AccessType;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { std::string("MemberAccessExpr:") + (AccessType == MemberAccessType::Direct ? "." : "->") + Member};
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};


class PostCrementExpression : public PostfixExpression {
public:
    enum class CrementType {
        Increment,
        Decrement,
    };

    explicit PostCrementExpression(
            NODE(Expr)& left,
            CrementType type
            ) : PostfixExpression(PostExprType::Crement) {
        this->Left = std::move(left);
        this-> Type = type;
    }

    NODE(Expr) Left;
    CrementType Type;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { std::string("PostCrementExpression:") + (Type == CrementType::Increment ? "++" : "--") };
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};


//class TypeInitializerExpression : public PostfixExpression {
//public:
//
//    explicit TypeInitializerExpression(
//            std::vector<Expr> args
//    ) : PostfixExpression(PostExprType::TypeInitializer) {
//        this->Args = std::move(args);
//    }
//
//    std::vector<Expr> Args;
//
//    std::vector<std::string> Repr() override {
//        std::vector<std::string> repr = { "TypeInitializerExpr:" };
//        for (auto& s : Args->Repr()) {
//            repr.emplace_back(REPR_PADDING + s);
//        }
//        return repr;
//    }
//};


#endif //EPICALYX_POSTFIX_EXPRESSION_H
