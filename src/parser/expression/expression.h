#ifndef EPICALYX_EXPRESSION_H
#define EPICALYX_EXPRESSION_H

#include <memory>
#include <utility>
#include "../AST.h"

class UnaryExpression;
class BinOpExpression;
class AssignmentExpression;

class Expression : public Expr {
public:
    Expression(std::shared_ptr<Expr> left, std::shared_ptr<Expr> right) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    std::shared_ptr<Expr> Left;
    std::shared_ptr<Expr> Right;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "Expr:" };
        for (auto& s : Left->Repr()) {
            repr.emplace_back(s);
        }
        repr.emplace_back("Expr:");
        for (auto& s : Right->Repr()) {
            repr.emplace_back(s);
        }
        return repr;
    }
};

class AssignmentExpression : public Expr {
public:
    enum class AssignOp {
        Eq,
        MulEq,
        DivEq,
        ModEq,
        AddEq,
        SubEq,
        LShiftEq,
        RShiftEq,
        AndEq,
        XorEq,
        OrEq,
    };

    AssignmentExpression(
            std::shared_ptr<Expr> left,
            AssignOp op,
            std::shared_ptr<Expr> right) {
        this->Left = std::move(left);
        this->Op = op;
        this->Right = std::move(right);
    }

    std::shared_ptr<Expr> Left;
    AssignOp Op;
    std::shared_ptr<Expr> Right;  // can also be a CondExpr

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "AssignExpr:" };
        for (auto& s : Left->Repr()) {
            repr.emplace_back("    " + s);
        }
        repr.emplace_back(Operation());
        for (auto& s : Right->Repr()) {
            repr.emplace_back("    " + s);
        }
        return repr;
    }

private:

    std::string Operation() {
        switch(Op) {
            case AssignOp::Eq:
                return "=";
            case AssignOp::MulEq:
                return "*=";
            case AssignOp::DivEq:
                return "/=";
            case AssignOp::ModEq:
                return "%=";
            case AssignOp::AddEq:
                return "+=";
            case AssignOp::SubEq:
                return "-=";
            case AssignOp::LShiftEq:
                return "<<=";
            case AssignOp::RShiftEq:
                return ">>=";
            case AssignOp::AndEq:
                return "&=";
            case AssignOp::XorEq:
                return "^=";
            case AssignOp::OrEq:
                return "|=";
        }
    }
};

#endif //EPICALYX_EXPRESSION_H
