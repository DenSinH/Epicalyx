#ifndef EPICALYX_STATIC_ASSERT_H
#define EPICALYX_STATIC_ASSERT_H

#include "../AST.h"
#include "../expression/primary_expression.h"
#include "struct.h"
#include <stdexcept>

class StaticAssertDecl : public StructDeclaration {
public:
    StaticAssertDecl(NODE(Expr)& expression, std::string& message) {
        Expression = std::move(expression);
        Message = message;

        if (!Expression->IsConstant()) {
            throw std::runtime_error("Non constant expression in static assert expression");
        }
    }

    NODE(Expr) Expression;  // must be constant
    std::string Message;

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "StaticAssert: " };
        for (auto& s : Expression->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        repr.push_back("Message: " + Message);
        return repr;
    }
};

#endif //EPICALYX_STATIC_ASSERT_H