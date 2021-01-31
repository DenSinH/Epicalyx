#include "parser.h"
#include "statement_nodes.h"

NODE(StatementNode) Parser::ExpectStatement() {
    auto current = Current();
    switch (current->Type) {
        case TokenType::Continue: {
            // continue ;
            Advance();
            EatType(TokenType::SemiColon);
            return MAKE_NODE(Continue)(current);
        }
        case TokenType::Break: {
            // break ;
            Advance();
            EatType(TokenType::SemiColon);
            return MAKE_NODE(Break)(current);
        }
        case TokenType::Goto: {
            // goto identifier ;
            Advance();  // goto
            ExpectType(TokenType::Identifier);
            std::string label = std::static_pointer_cast<Identifier>(Current())->Name;
            Advance();  // identifier
            EatType(TokenType::SemiColon);
            return MAKE_NODE(Goto)(current, label);
        }
        case TokenType::Return: {
            // return [expression] ;
            Advance();
            if (Current()->Type == TokenType::SemiColon) {
                Advance();
                return MAKE_NODE(Return)(current);
            }
            auto expr = ExpectExpression();
            EatType(TokenType::SemiColon);
            return MAKE_NODE(Return)(current, std::move(expr));
        }
        case TokenType::Case: {
            Advance();
            auto expr = ExpectConstantExpression();
            EatType(TokenType::Colon);
            auto statement = ExpectStatement();
            return MAKE_NODE(CaseStatement)(current, std::move(expr), std::move(statement));
        }
        case TokenType::Default: {
            Advance();
            EatType(TokenType::Colon);
            auto statement = ExpectStatement();
            return MAKE_NODE(DefaultStatement)(current, std::move(statement));
        }
        case TokenType::If: {
            // if statement
            // if ( expression ) statement [else statement]
            Advance();
            EatType(TokenType::LParen);
            auto condition = ExpectExpression();
            EatType(TokenType::LParen);
            auto statement = ExpectStatement();
            if (!EndOfStream() && Current()->Type == TokenType::Else) {
                Advance();
                auto _else = ExpectStatement();
                return MAKE_NODE(IfStatement)(current, std::move(condition), std::move(statement), std::move(_else));
            }
            return MAKE_NODE(IfStatement)(current, std::move(condition), std::move(statement));
        }
        case TokenType::Switch: {
            Advance();
            EatType(TokenType::LParen);
            auto expression = ExpectExpression();
            EatType(TokenType::RParen);
            auto statement = ExpectStatement();
            return MAKE_NODE(Switch)(current, std::move(expression), std::move(statement));
        }
        case TokenType::While: {
            Advance();
            EatType(TokenType::LParen);
            auto condition = ExpectExpression();
            EatType(TokenType::RParen);
            auto statement = ExpectStatement();
            return MAKE_NODE(While)(current, std::move(condition), std::move(statement));
        }
        case TokenType::Do: {
            Advance();
            auto statement = ExpectStatement();
            EatType(TokenType::While);
            EatType(TokenType::LParen);
            auto condition = ExpectExpression();
            EatType(TokenType::RParen);
            EatType(TokenType::SemiColon);
            return MAKE_NODE(DoWhile)(current, std::move(statement), std::move(condition));
        }
        case TokenType::For: {
            Advance();
            EatType(TokenType::LParen);
            NODE(DeclNode) declaration = nullptr;
            NODE(ExprNode) initializer = nullptr;

            // declaration and initializer are optional
            if (Current()->Type != TokenType::SemiColon) {
                if (IsDeclarationSpecifier(Current())) {
                    // declaration
                    declaration = ExpectDeclaration();
                }
                else {
                    // todo: the grammar says otherwise, but that doesn't seem right
                    // grammar says: declaration expression[opt]
                    initializer = ExpectExpression();
                    EatType(TokenType::SemiColon);
                }
            }
            else {
                EatType(TokenType::SemiColon);
            }
            // condition is also optional
            NODE(ExprNode) condition = nullptr;
            if (Current()->Type != TokenType::SemiColon) {
                condition = ExpectExpression();
            }
            EatType(TokenType::SemiColon);

            // updation is also optional
            NODE(ExprNode) updation = nullptr;
            if (Current()->Type != TokenType::RParen) {
                updation = ExpectExpression();
            }
            EatType(TokenType::RParen);
            auto statement = ExpectStatement();
            return MAKE_NODE(For)(
                    current,
                    std::move(declaration),
                    std::move(initializer),
                    std::move(condition),
                    std::move(updation),
                    std::move(statement)
            );
        }
        case TokenType::SemiColon: {
            // empty statement
            Advance();
            return MAKE_NODE(EmptyStatement)(current);
        }
        case TokenType::Identifier: {
            std::string name = std::static_pointer_cast<Identifier>(current)->Name;
            if (Next()->Type == TokenType::Colon) {
                // labeled statement
                Advance();  // identifier
                Advance();  // colon
                auto statement = ExpectStatement();
                return MAKE_NODE(LabelStatement)(current, name, std::move(statement));
            }
            // if it's not a labeled statement, it has to be an expression
            // declarations are not statements, but block items
            NODE(StatementNode) expr = ExpectExpression();
            EatType(TokenType::SemiColon);
            return expr;
        }
        case TokenType::LBrace: {
            // compound statement
            return ExpectCompoundStatement();
        }
        default: {
            throw std::runtime_error("Unexpected token: " + current->Repr());
        }
    }
}


NODE(CompoundStatement) Parser::ExpectCompoundStatement() {
    auto current = Current();
    EatType(TokenType::LBrace);

    auto compound = MAKE_NODE(CompoundStatement)(current);
    while (Current()->Type != TokenType::RBrace) {
        if (IsDeclarationSpecifier(Current())) {
            compound->AddStatement(ExpectDeclaration());
        }
        else {
            compound->AddStatement(ExpectStatement());
        }
    }
    EatType(TokenType::RBrace);
    return compound;
}
