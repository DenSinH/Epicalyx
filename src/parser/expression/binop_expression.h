#ifndef EPICALYX_BINOP_EXPRESSION_H
#define EPICALYX_BINOP_EXPRESSION_H

#include <memory>
#include <utility>
#include "log.h"
#include "../AST.h"

class CondExpr : public Expr {
public:
    CondExpr(std::unique_ptr<Expr>& left,
             std::unique_ptr<Expr>& t,
             std::unique_ptr<Expr>& f) {
        this->Left = std::move(left);
        this->True = std::move(t);
        this->False = std::move(f);
    }

    std::unique_ptr<Expr> Left;
    std::unique_ptr<Expr> True;
    std::unique_ptr<Expr> False;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "CondExpr: Cond:" };
        for (auto& s : Left->Repr()) {
            repr.emplace_back("    " + s);
        }
        repr.emplace_back("True:");
        for (auto& s : True->Repr()) {
            repr.emplace_back("    " + s);
        }
        repr.emplace_back("False:");
        for (auto& s : False->Repr()) {
            repr.emplace_back("    " + s);
        }
        return repr;
    }
};


class BinOpExpression : public Expr {
public:
    enum class BinOp {
        Mul,
        Div,
        Mod,
        Add,
        Sub,
        LShift,
        RShift,
        Leq,
        Geq,
        Lt,
        Gt,
        Eq,
        Ne,
        BinAnd,
        BinXOr,
        BinOr,
        LogicAnd,
        LogicOr,
    };

    BinOpExpression(BinOp op, std::unique_ptr<Expr>& left, std::unique_ptr<Expr>& right) {
        this->Op = op;
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    BinOp Op;
    std::unique_ptr<Expr> Left;
    std::unique_ptr<Expr> Right;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "BinOp: " + Operation() };
        for (auto& s : Left->Repr()) {
            repr.emplace_back("    " + s);
        }
        for (auto& s : Right->Repr()) {
            repr.emplace_back("    " + s);
        }
        return repr;
    }

private:
    std::string Operation() {
        switch(Op) {
            case BinOp::Add:
                return "+";
            case BinOp::Mul:
                return "*";
            case BinOp::Div:
                return "/";
            case BinOp::Mod:
                return "%";
            case BinOp::Sub:
                return "-";
            case BinOp::LShift:
                return "<<";
            case BinOp::RShift:
                return ">>";
            case BinOp::Leq:
                return "<=";
            case BinOp::Geq:
                return ">=";
            case BinOp::Lt:
                return "<";
            case BinOp::Gt:
                return ">";
            case BinOp::Eq:
                return "==";
            case BinOp::Ne:
                return "!=";
            case BinOp::BinAnd:
                return "&";
            case BinOp::BinXOr:
                return "^";
            case BinOp::BinOr:
                return "|";
            case BinOp::LogicAnd:
                return "&&";
            case BinOp::LogicOr:
                return "||";
            default:
                log_fatal("Invalid Binop operation");
        }
    }

};

#endif //EPICALYX_BINOP_EXPRESSION_H
