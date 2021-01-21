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
    virtual std::list<std::string> Repr() { return {}; }

    void Print() {
        for (auto& s : Repr()) {
            printf("%s\n", s.c_str());
        }
    }

};

class Expr : public Node {
public:
    virtual bool IsConstant() {
        return false;
    }
};

class Decl : public Node {

};

#endif //EPICALYX_AST_H
