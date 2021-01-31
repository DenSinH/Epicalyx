#include "parser.h"
#include "translation_unit/function_definition.h"

NODE(Node) Parser::ExpectTranslationUnit() {
    auto current = Current();
    auto specifiers = ExpectDeclarationSpecifiers();
    auto declarator = ExpectDeclarator<Declarator>();

    /*
     * 2 cases:
     *  - function definition: followed by {  todo: declaration-list in spec?
     *  - declaration: followed by = or , or ;
     * */
    if (declarator->IsFunction() && Current()->Type == TokenType::LBrace) {
        // function definition
//        Advance();
//        std::vector<NODE(Declaration)> args = {};
//        if (Current()->Type != TokenType::RParen) {
//            do {
//                // argument declarations
//                auto arg = ExpectDeclaration();
//                args.emplace_back(std::move(arg));
//                if (Current()->Type != TokenType::RParen) {
//                    EatType(TokenType::Comma);
//                }
//                else {
//                    break;
//                }
//            } while (true);
//        }
//        EatType(TokenType::RParen);
        auto body = ExpectCompoundStatement();
        return MAKE_NODE(FunctionDefinition)(current, std::move(specifiers), std::move(declarator), std::move(body));
    }
    // declaration
    // code below is the same as ExpectDeclaration
    auto declaration = MAKE_NODE(Declaration)(Current(), std::move(specifiers));
    do {
        // init-declarator-list
        // normally we would find a declarator here, but we have already found it, so we look for it at the end
        current = Current();
        if (current->Type == TokenType::Assign) {
            Advance();
            auto initializer = ExpectInitializer();
            auto init_declarator = MAKE_NODE(InitDeclarator)(current, std::move(declarator), std::move(initializer));
            declaration->AddDeclarator(init_declarator);
        }
        else {
            auto init_declarator = MAKE_NODE(InitDeclarator)(current, std::move(declarator));
            declaration->AddDeclarator(init_declarator);
        }

        if (Current()->Type != TokenType::SemiColon) {
            EatType(TokenType::Comma);
        }
        else {
            break;
        }
        declarator = ExpectDeclarator<Declarator>();
    } while (true);

    EatType(TokenType::SemiColon);
    return declaration;
}