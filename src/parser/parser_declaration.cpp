#include "parser.h"

#include "declaration_nodes.h"


NODE(StaticAssertDecl) Parser::ExpectStaticAssert() {
    EatType(TokenType::StaticAssert);
    EatType(TokenType::LParen);
    auto expr = ExpectConstantExpression();
    EatType(TokenType::Comma);
    ExpectType(TokenType::ConstString);
    std::string message = std::static_pointer_cast<StringConstant>(Current())->Value;
    EatType(TokenType::ConstString);
    EatType(TokenType::RParen);
    EatType(TokenType::SemiColon);
    return MAKE_NODE(StaticAssertDecl)(expr, message);
}


NODE(Initializer) Parser::ExpectInitializer() {
    auto current = Current();
    if (current->Type != TokenType::LBrace) {
        // assignment expression initializer
        auto expr = ExpectAssignmentExpression();
        return MAKE_NODE(AssignmentInitializer)(expr);
    }
    // initializer list
    auto initializer = MAKE_NODE(InitializerList)();
    EatType(TokenType::LBrace);

    // { (([constexpr] / .identifier)+ =)? initializer (,)? }
    while (true) {
        // list might end in a comma, we checked this at the end of the loop
        current = Current();

        // end of initializer list
        if (current->Type == TokenType::RBrace) {
            EatType(TokenType::RBrace);
            return initializer;
        }

        // find all designators (if any)
        std::vector<NODE(Designator)> designators = {};
        for (current = Current(); Is(current->Type).AnyOf<TokenType::Dot, TokenType::LBracket>(); current = Current()) {
            Advance();
            if (current->Type == TokenType::Dot) {
                // struct member initializer .identifier
                ExpectType(TokenType::Identifier);
                std::string name = std::static_pointer_cast<Identifier>(Current())->Name;
                designators.push_back(MAKE_NODE(StructFieldDesignator)(name));
                Advance();
            }
            else {
                // array field initializer [constant-expression]
                auto expr = ExpectConstantExpression();
                designators.push_back(MAKE_NODE(ArrayMemberDesignator)(expr));
                EatType(TokenType::RBracket);
            }
        }

        // only an = if there are any designators
        if (!designators.empty()) {
            EatType(TokenType::Assign);
        }

        auto designation_initializer = ExpectInitializer();
        initializer->AddInitializer(designators, designation_initializer);

        // either a comma and another initializer-list or end of initializer
        if (Current()->Type != TokenType::RBrace) {
            EatType(TokenType::Comma);
        }
    } while (true);
}
