#ifndef EPICALYX_AST_H
#define EPICALYX_AST_H

#include <vector>
#include <string>
#include <memory>

#define REPR_PADDING "|   "

class Parser;

class Node {
public:
    virtual std::vector<std::string> Repr() { return {}; }

    void Print() {
        for (auto& s : Repr()) {
            printf("%s\n", s.c_str());
        }
    }

};

class Expr : public Node {

};

#endif //EPICALYX_AST_H
