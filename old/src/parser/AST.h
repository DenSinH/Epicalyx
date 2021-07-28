#ifndef EPICALYX_AST_H
#define EPICALYX_AST_H

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <utility>
#include <stdexcept>

#include "../tokenizer/tokens.h"
#include "../state/state.h"
#include "types/types.h"

#define REPR_PADDING "|   "
#define NODE(...) std::unique_ptr<__VA_ARGS__>
#define MAKE_NODE(...) std::make_unique<__VA_ARGS__>

class Parser;
class ParserState;

class Analyzable {
public:
    virtual CTYPE SemanticAnalysis(const ParserState& state) const {
        throw std::runtime_error("Not implemented");
    }
};

class Node : public InFile {
public:
    explicit Node(const TOKEN& tok) : InFile(*tok) {}

    [[nodiscard]] virtual std::list<std::string> Repr() const { return {}; }

    void Print() const {
        for (auto& s : Repr()) {
            printf("%s\n", s.c_str());
        }
    }

protected:
    template<typename N>
    static void NestedRepr(std::list<std::string>& repr, const NODE(N)& node) {
        for (auto& s : node->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
    }

    template<typename N>
    static void NestedRepr(std::list<std::string>& repr, const std::vector<NODE(N)>& nodes) {
        for (auto& n : nodes) {
            for (auto& s : n->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
    }
};

class BlockItem : public Node, public Analyzable {
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

    virtual bool IsConstant(const ParserState& state) const {
        return false;
    }

    virtual CTYPE ConstEval(const ParserState& state) const {
        throw std::runtime_error("Expression is not a constant");
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

class TypeSpecifierNode : public Node {
public:
    explicit TypeSpecifierNode(const TOKEN& tok) : Node(tok) {}
};

class DeclNode : public BlockItem {
public:
    explicit DeclNode(const TOKEN& tok) : BlockItem(tok) {}
};

#endif //EPICALYX_AST_H