#ifndef EPICALYX_ASSIGN_EXPRESSION_H
#define EPICALYX_ASSIGN_EXPRESSION_H

#include "../AST.h"
#include "../../tokenizer/types.h"

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
            NODE(Expr)& left,
            AssignOp op,
            NODE(Expr)& right) {
        this->Left = std::move(left);
        this->Op = op;
        this->Right = std::move(right);
    }

    NODE(Expr) Left;
    AssignOp Op;
    NODE(Expr) Right;  // can also be a CondExpr

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "AssignExpr:" };
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        repr.emplace_back(Operation());
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }

    static constexpr AssignOp TokenTypeToAssignOp(enum TokenType type) {
        switch(type) {
            case TokenType::Assign:
                return AssignOp::Eq;
            case TokenType::IMul:
                return AssignOp::MulEq;
            case TokenType::IDiv:
                return AssignOp::DivEq;
            case TokenType::IMod:
                return AssignOp::ModEq;
            case TokenType::IPlus:
                return AssignOp::AddEq;
            case TokenType::IMinus:
                return AssignOp::SubEq;
            case TokenType::ILShift:
                return AssignOp::LShiftEq;
            case TokenType::IRShift:
                return AssignOp::RShiftEq;
            case TokenType::IAnd:
                return AssignOp::AndEq;
            case TokenType::IXor:
                return AssignOp::XorEq;
            case TokenType::IOr:
                return AssignOp::OrEq;
            default:
                throw std::runtime_error("Invalid assignment token type");
        }
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


#endif //EPICALYX_ASSIGN_EXPRESSION_H
