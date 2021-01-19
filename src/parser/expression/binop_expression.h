#ifndef EPICALYX_BINOP_EXPRESSION_H
#define EPICALYX_BINOP_EXPRESSION_H

#include <memory>
#include <utility>
#include <stdexcept>
#include "log.h"
#include "../AST.h"

class CondExpr : public Expr {
public:
    CondExpr(NODE(Expr)& left,
             NODE(Expr)& t,
             NODE(Expr)& f) {
        this->Left = std::move(left);
        this->True = std::move(t);
        this->False = std::move(f);
    }

    NODE(Expr) Left;
    NODE(Expr) True;
    NODE(Expr) False;

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
        BinXor,
        BinOr,
        LogicAnd,
        LogicOr,
    };

    BinOpExpression(BinOp op, NODE(Expr)& left, NODE(Expr)& right) {
        this->Op = op;
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    BinOp Op;
    NODE(Expr) Left;
    NODE(Expr) Right;

    static constexpr BinOp TokenTypeToBinOp(const enum TokenType type) {
        switch(type) {
            case TokenType::Asterisk:
                return BinOp::Mul;
            case TokenType::Div:
                return BinOp::Div;
            case TokenType::Mod:
                return BinOp::Mod;
            case TokenType::Plus:
                return BinOp::Add;
            case TokenType::Minus:
                return BinOp::Sub;
            case TokenType::LShift:
                return BinOp::LShift;
            case TokenType::RShift:
                return BinOp::RShift;
            case TokenType::LessEqual:
                return BinOp::Leq;
            case TokenType::GreaterEqual:
                return BinOp::Geq;
            case TokenType::Less:
                return BinOp::Lt;
            case TokenType::Greater:
                return BinOp::Gt;
            case TokenType::Equal:
                return BinOp::Eq;
            case TokenType::NotEqual:
                return BinOp::Ne;
            case TokenType::Ampersand:
                return BinOp::BinAnd;
            case TokenType::BinOr:
                return BinOp::BinOr;
            case TokenType::BinXor:
                return BinOp::BinXor;
            case TokenType::LogicalAnd:
                return BinOp::LogicAnd;
            case TokenType::LogicalOr:
                return BinOp::LogicOr;
            default:
                throw std::runtime_error("Invalid BinOp token");
        }
    }

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
            case BinOp::BinXor:
                return "^";
            case BinOp::BinOr:
                return "|";
            case BinOp::LogicAnd:
                return "&&";
            case BinOp::LogicOr:
                return "||";
            default:
                throw std::runtime_error("Invalid Binop operation");
        }
    }

};

#endif //EPICALYX_BINOP_EXPRESSION_H
