#ifndef EPICALYX_UNARY_EXPRESSION_H
#define EPICALYX_UNARY_EXPRESSION_H

#include "../AST.h"
#include "../declaration/typename.h"

// a cast expression can just be a unary expression
class UnaryExpression : public ExprNode {
public:
    enum class UnExprType {
        PostFix,
        UnOp,  // ++/--/&/*/+/-/~/!
        SizeOf,
        AlignOf,
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

    explicit UnaryOpExpression(UnOpType type, NODE(ExprNode)& right) : UnaryExpression(UnExprType::UnOp) {
        this->Type = type;
        this->Right = std::move(right);
    }

    UnOpType Type;
    NODE(ExprNode) Right;

    bool IsConstant() override {
        return Right->IsConstant();
    }

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "UnaryExpression: " + Operation() };
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
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

class SizeOfTypeExpression : public UnaryExpression {
public:
    explicit SizeOfTypeExpression(NODE(TypeName)& right) : UnaryExpression(UnExprType::SizeOf) {
        this->Right = std::move(right);
    }

    NODE(TypeName) Right;

    bool IsConstant() override {
        return true;
    }

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "SizeOfTypeExpression: " };
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class SizeOfExpression : public UnaryExpression {
public:
    explicit SizeOfExpression(NODE(ExprNode)& right) : UnaryExpression(UnExprType::SizeOf) {
        this->Right = std::move(right);
    }

    NODE(ExprNode) Right;
    bool IsConstant() override {
        return true;
    }

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "SizeOfExpression: " };
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class AlignOfExpression : public UnaryExpression {
public:
    explicit AlignOfExpression(NODE(TypeName)& right) : UnaryExpression(UnExprType::AlignOf) {
        this->Right = std::move(right);
    }

    NODE(TypeName) Right;

    bool IsConstant() override {
        return true;
    }

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "AlignOf: " };
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};

#endif //EPICALYX_UNARY_EXPRESSION_H
