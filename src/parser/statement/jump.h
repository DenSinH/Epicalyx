#ifndef EPICALYX_JUMP_H
#define EPICALYX_JUMP_H

#include "AST.h"

class JumpStatement : public StatementNode {
public:
    explicit JumpStatement(const TOKEN& tok) : StatementNode(tok) {

    }
};

class Goto : public JumpStatement {
public:
    explicit Goto(const TOKEN& tok, const std::string& label) :
            JumpStatement(tok),
            Label(label) {

    }

    const std::string Label;

    std::list<std::string> Repr() const override {
        return { "goto: " + Label };
    }
};

class Continue : public JumpStatement {
public:
    explicit Continue(const TOKEN& tok) : JumpStatement(tok) {

    }

    std::list<std::string> Repr() const override {
        return { "continue" };
    }
};

class Break : public JumpStatement {
public:
    explicit Break(const TOKEN& tok) : JumpStatement(tok) {

    }

    std::list<std::string> Repr() const override {
        return { "break" };
    }
};

class Return : public JumpStatement {
public:
    explicit Return(const TOKEN& tok) :
            JumpStatement(tok),
            Expression(nullptr) {

    }

    explicit Return(const TOKEN& tok, NODE(ExprNode)& expression) :
            JumpStatement(tok),
            Expression(std::move(expression)) {

    }

    const NODE(ExprNode) Expression;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "return: " };
        for (auto& s : Expression->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        return repr;
    }
};

#endif //EPICALYX_JUMP_H
