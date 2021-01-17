#ifndef EPICALYX_UNARY_EXPRESSION_H
#define EPICALYX_UNARY_EXPRESSION_H


#include <memory>
#include <utility>
#include <vector>
#include "../AST.h"


// a cast expression can just be a unary expression
class UnaryExpression : public Expr {
public:
    enum class UnExprType {
        PostFix,
        UnOp,  // ++/--/&/*/+/-/~/!
        SizeOf,
    };

    explicit UnaryExpression(UnExprType type) {
        this->Type = type;
    }

    UnExprType Type;
};

class UnaryOpExpression : public UnaryExpression {
public:
    enum class UnOpType {
        PreIncrement,
        PreDecrement,
        Reference,
        Dereference,
        Positive,
        Negative,
        BinaryNot,
        LogicalNot,
    };

    explicit UnaryOpExpression(UnOpType type, std::unique_ptr<Expr> right) : UnaryExpression(UnExprType::UnOp) {
        this->Type = type;
        this->Right = std::move(right);
    }

    UnOpType Type;
    std::unique_ptr<Expr> Right;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "UnaryExpression: " + Operation() };
        for (auto& s : Right->Repr()) {
            repr.emplace_back("    " + s);
        }
        return repr;
    }

private:
    std::string Operation() {
        switch(Type) {
            case UnOpType::PreIncrement:
                return "++";
            case UnOpType::PreDecrement:
                return "--";
            case UnOpType::Reference:
                return "&";
            case UnOpType::Dereference:
                return "*";
            case UnOpType::Positive:
                return "+";
            case UnOpType::Negative:
                return "-";
            case UnOpType::BinaryNot:
                return "~";
            case UnOpType::LogicalNot:
                return "!";
        }
    }
};

class SizeOfExpression : public UnaryExpression {

    explicit SizeOfExpression(std::unique_ptr<Expr> right) : UnaryExpression(UnExprType::SizeOf) {
        this->Right = std::move(right);
    }

    std::unique_ptr<Expr> Right;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "SizeOfExpression: " };
        for (auto& s : Right->Repr()) {
            repr.emplace_back("    " + s);
        }
        return repr;
    }
};

#endif //EPICALYX_UNARY_EXPRESSION_H
