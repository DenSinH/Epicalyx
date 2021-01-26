#ifndef EPICALYX_AST_H
#define EPICALYX_AST_H

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <utility>

#include "../tokenizer/tokens.h"
#include "../state/state.h"

#define REPR_PADDING "|   "
#define NODE(_type) std::unique_ptr<_type>
#define MAKE_NODE(_type) std::make_unique<_type>

class Parser;

class Node : public InFile {
public:
    explicit Node(const TOKEN& tok) : InFile(*tok) {}

    [[nodiscard]] virtual std::list<std::string> Repr() const { return {}; }

    void Print() const {
        for (auto& s : Repr()) {
            printf("%s\n", s.c_str());
        }
    }
};

class BlockItem : public Node {
public:
    explicit BlockItem(const TOKEN& tok) : Node(tok) {}
};

class StatementNode : public BlockItem {
public:
    explicit StatementNode(const TOKEN& tok) : BlockItem(tok) {}
};

class ExprNode : public StatementNode {
public:
    explicit ExprNode(const TOKEN& tok) : StatementNode(tok) {}

    virtual bool IsConstant() const {
        return false;
    }
};

class SpecifierQualifier : public Node {
public:
    explicit SpecifierQualifier(const TOKEN& tok) : Node(tok) {}

    [[nodiscard]] virtual std::string String() const { return ""; }

    [[nodiscard]] std::list<std::string> Repr() const override {
        return { String() };
    }
};

class TypeNode : public Node {
public:
    explicit TypeNode(const TOKEN& tok) : Node(tok) {}
};

class DeclNode : public BlockItem {
public:
    explicit DeclNode(const TOKEN& tok) : BlockItem(tok) {}
};

#endif //EPICALYX_AST_H
