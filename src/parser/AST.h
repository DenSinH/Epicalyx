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
#define NODE(_type) std::unique_ptr<_type>
#define MAKE_NODE(_type) std::make_unique<_type>

class Parser;
class ParserState;

class Typed {
public:
    virtual TYPE GetType(const ParserState& state) const {
        throw std::runtime_error("Not implemented");
    }
};

class Analyzable {
public:
    virtual bool SemanticAnalysis(ParserState& state) {
        return true;
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

class BlockItem : public Node {
public:
    explicit BlockItem(const TOKEN& tok) : Node(tok) {}
};

class StatementNode : public BlockItem {
public:
    explicit StatementNode(const TOKEN& tok) : BlockItem(tok) {}
};

class ExprNode : public StatementNode, public Typed, public Analyzable {
public:
    explicit ExprNode(const TOKEN& tok) : StatementNode(tok) {}

    virtual bool IsConstant(const ParserState& state) const {
        return false;
    }

    virtual TYPE ConstEval(const ParserState& state) const {
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

class DeclNode : public BlockItem, Typed, Analyzable {
public:
    explicit DeclNode(const TOKEN& tok) : BlockItem(tok) {}
};

#endif //EPICALYX_AST_H
