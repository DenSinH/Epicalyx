#ifndef EPICALYX_STATIC_ASSERT_H
#define EPICALYX_STATIC_ASSERT_H

#include "../AST.h"
#include "../expression/primary_expression.h"
#include "struct.h"
#include <stdexcept>

class StaticAssertDecl : public StructDeclaration {
public:
    StaticAssertDecl(const TOKEN& tok, NODE(ExprNode)&& expression, const std::string& message) :
            StructDeclaration(tok),
            Expression(std::move(expression)),
            Message(message) {

//        if (!Expression->IsConstant(<#initializer#>)) {
//            throw std::runtime_error("Non constant expression in static assert expression");
//        }
    }

    const NODE(ExprNode) Expression;  // must be constant
    const std::string Message;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "StaticAssert: " };
        NestedRepr(repr, Expression);
        repr.push_back("Message: " + Message);
        return repr;
    }
};

#endif //EPICALYX_STATIC_ASSERT_H
