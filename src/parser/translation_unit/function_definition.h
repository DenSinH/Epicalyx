#ifndef EPICALYX_FUNCTION_DEFINITION_H
#define EPICALYX_FUNCTION_DEFINITION_H

#include "AST.h"
#include "../statement/compound.h"
#include "../declaration/declaration.h"

class FunctionDefinition : public Node {
public:
    FunctionDefinition(
            const TOKEN& tok,
            NODE(DeclarationSpecifiers)&& specifiers,
            NODE(Declarator)&& decl,
            NODE(CompoundStatement)&& body
            ) :
                Node(tok),
                Specifiers(std::move(specifiers)),
                Decl(std::move(decl)),
                Body(std::move(body)) {

    }

    const NODE(DeclarationSpecifiers) Specifiers;
    const NODE(Declarator) Decl;
    const NODE(CompoundStatement) Body;


    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "function: "};
        NestedRepr(repr, Specifiers);
        repr.emplace_back("declarator:");
        NestedRepr(repr, Decl);

        repr.emplace_back("body:");
        NestedRepr(repr, Body);
        return repr;
    }
};

#endif //EPICALYX_FUNCTION_DEFINITION_H
