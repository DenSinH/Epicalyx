#ifndef EPICALYX_LABELED_H
#define EPICALYX_LABELED_H

#include "AST.h"

class LabeledStatement : public StatementNode {
public:
    explicit LabeledStatement(const TOKEN& tok, NODE(StatementNode)&& statement) :
            StatementNode(tok),
            Statement(std::move(statement)) {

    }

    const NODE(StatementNode) Statement;
};

class LabelStatement : public LabeledStatement {
public:
    explicit LabelStatement(const TOKEN& tok, const std::string& label, NODE(StatementNode)&& statement) :
            LabeledStatement(tok, std::move(statement)),
            Label(label) {

    }

    const std::string Label;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "label: " + Label};
        for (auto& s : Statement->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class CaseStatement : public LabeledStatement {
public:
    explicit CaseStatement(const TOKEN& tok, NODE(ExprNode)&& expression, NODE(StatementNode)&& statement) :
            LabeledStatement(tok, std::move(statement)),
            Expression(std::move(expression)) {

    }

    const NODE(ExprNode) Expression;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "case: "};
        for (auto& s : Expression->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }

        repr.emplace_back("statement:");
        for (auto& s : Statement->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class DefaultStatement : public LabeledStatement {
public:
    explicit DefaultStatement(const TOKEN& tok, NODE(StatementNode)&& statement) :
            LabeledStatement(tok, std::move(statement)) {

    }

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "default: "};
        for (auto& s : Statement->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        return repr;
    }
};

#endif //EPICALYX_LABELED_H
