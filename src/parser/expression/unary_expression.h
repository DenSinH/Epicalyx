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

    explicit UnaryExpression(const TOKEN& tok, UnExprType type) : ExprNode(tok) {
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

    explicit UnaryOpExpression(const TOKEN& tok, UnOpType type, NODE(ExprNode)& right) : UnaryExpression(tok, UnExprType::UnOp) {
        this->Type = type;
        this->Right = std::move(right);
    }

    UnOpType Type;
    NODE(ExprNode) Right;

    bool IsConstant() const override {
        return Right->IsConstant();
    }

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "UnaryExpression: " + Operation() };
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }

private:
    std::string Operation() const {
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
    explicit SizeOfTypeExpression(const TOKEN& tok, NODE(TypeName)& right) : UnaryExpression(tok, UnExprType::SizeOf) {
        this->Right = std::move(right);
    }

    NODE(TypeName) Right;

    bool IsConstant() const override {
        return true;
    }

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "SizeOfTypeExpression: " };
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class SizeOfExpression : public UnaryExpression {
public:
    explicit SizeOfExpression(const TOKEN& tok, NODE(ExprNode)& right) : UnaryExpression(tok, UnExprType::SizeOf) {
        this->Right = std::move(right);
    }

    NODE(ExprNode) Right;
    bool IsConstant() const override {
        return true;
    }

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "SizeOfExpression: " };
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class AlignOfExpression : public UnaryExpression {
public:
    explicit AlignOfExpression(const TOKEN& tok, NODE(TypeName)& right) : UnaryExpression(tok, UnExprType::AlignOf) {
        this->Right = std::move(right);
    }

    NODE(TypeName) Right;

    bool IsConstant() const override {
        return true;
    }

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "AlignOf: " };
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }
};

#endif //EPICALYX_UNARY_EXPRESSION_H
