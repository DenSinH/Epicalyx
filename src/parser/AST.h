#ifndef EPICALYX_AST_H
#define EPICALYX_AST_H

#include <string>

class Node {

    virtual std::string Repr() { return ""; }
};

class Expr : public Node {

};

#endif //EPICALYX_AST_H
