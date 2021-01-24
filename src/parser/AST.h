#ifndef EPICALYX_AST_H
#define EPICALYX_AST_H

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <utility>

#define REPR_PADDING "|   "
#define NODE(_type) std::unique_ptr<_type>
#define MAKE_NODE(_type) std::make_unique<_type>

class Parser;

class Node {
public:
    [[nodiscard]] virtual std::list<std::string> Repr() const { return {}; }

    void Print() const {
        for (auto& s : Repr()) {
            printf("%s\n", s.c_str());
        }
    }

};

class ExprNode : public Node {
public:
    virtual bool IsConstant() const {
        return false;
    }
};

class SpecifierQualifier : public Node {
public:
    [[nodiscard]] virtual std::string String() const { return ""; }

    [[nodiscard]] std::list<std::string> Repr() const override {
        return { String() };
    }
};

class TypeNode : public Node {

};

class DeclNode : public Node {

};

#endif //EPICALYX_AST_H
