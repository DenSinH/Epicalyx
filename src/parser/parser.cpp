#include "parser.h"
#include "expression/binop_expression.h"
#include "expression/cast_expression.h"
#include "expression/expression.h"
#include "expression/postfix_expression.h"
#include "expression/primary_expression.h"
#include "expression/unary_expression.h"


std::unique_ptr<Node> Parser::Parse() {
    return nullptr;
}

std::unique_ptr<Expr> Parser::ExpectPrimaryExpression() {
    auto current = Current();
    switch (current->Class) {
        case TokenClass::Identifier: {
            Advance();
            return std::make_unique<PrimaryExpressionIdentifier>(std::static_pointer_cast<Identifier>(current)->Name);
        }
        case TokenClass::NumericalConstant: {
            Advance();
            switch(current->Type) {
                case TokenType::ConstInt:
                    return std::make_unique<PrimaryExpressionConstant<int>>(std::static_pointer_cast<NumericalConstant<int>>(current)->Value);
                case TokenType::ConstUnsignedInt:
                    return std::make_unique<PrimaryExpressionConstant<unsigned int>>(std::static_pointer_cast<NumericalConstant<unsigned int>>(current)->Value);
                case TokenType::ConstLong:
                    return std::make_unique<PrimaryExpressionConstant<long>>(std::static_pointer_cast<NumericalConstant<long>>(current)->Value);
                case TokenType::ConstUnsignedLong:
                    return std::make_unique<PrimaryExpressionConstant<unsigned long>>(std::static_pointer_cast<NumericalConstant<unsigned long>>(current)->Value);
                case TokenType::ConstLongLong:
                    return std::make_unique<PrimaryExpressionConstant<long long>>(std::static_pointer_cast<NumericalConstant<long long>>(current)->Value);
                case TokenType::ConstUnsignedLongLong:
                    return std::make_unique<PrimaryExpressionConstant<unsigned long long>>(std::static_pointer_cast<NumericalConstant<unsigned long long>>(current)->Value);
                case TokenType::ConstFloat:
                    return std::make_unique<PrimaryExpressionConstant<float>>(std::static_pointer_cast<NumericalConstant<float>>(current)->Value);
                case TokenType::ConstDouble:
                    return std::make_unique<PrimaryExpressionConstant<double>>(std::static_pointer_cast<NumericalConstant<double>>(current)->Value);
                default:
                    log_fatal("Invalid token: numerical constant of unknown type");
            }
        }
        case TokenClass::StringConstant: {
            Advance();
            return std::make_unique<PrimaryStringLiteral>(std::static_pointer_cast<StringConstant>(current)->Value);
        }
        case TokenClass::Punctuator: {
            // has to be ( expression )
            EatType(TokenType::LParen);
            // todo: expression
        }
        default:
            log_fatal("Invalid syntax: expected primary expression, got keyword");
    }
}

std::unique_ptr<Expr> Parser::ExpectPostfixExpression() {
    auto current = Current();
    if (current->Type == TokenType::LParen) {
        log_fatal("Unimplemented: expect either primary expression or type initializer");
    }

    auto node = ExpectPrimaryExpression();
    while (!EndOfStream()) {
        current = Current();
        switch(current->Type) {
            case TokenType::LBracket: {
                // array access
                // todo: normal expression in parentheses, instead of postfix expression
                EatType(TokenType::LBracket);
                auto right = ExpectPostfixExpression();
                EatType(TokenType::RBracket);
                node = std::make_unique<ArrayAccessExpression>(node, right);
                break;
            }
            case TokenType::LParen: {
                // function call
                log_fatal("Unimplemented: function call expression");
            }
            case TokenType::Dot:
            case TokenType::Arrow: {
                // member access
                auto type = (current->Type == TokenType::Dot) ?
                        MemberAccessExpression::MemberAccessType::Direct :
                        MemberAccessExpression::MemberAccessType::Pointer;
                Advance();
                auto field = EatType(TokenType::Identifier);
                node = std::make_unique<MemberAccessExpression>(node, std::static_pointer_cast<Identifier>(field)->Name, type);
                break;
            }
            case TokenType::Incr:
            case TokenType::Decr: {
                // ++ / --
                auto type = (current->Type == TokenType::Incr) ?
                        PostCrementExpression::CrementType::Increment :
                        PostCrementExpression::CrementType::Decrement;
                Advance();
                node = std::make_unique<PostCrementExpression>(node, type);
                break;
            }
            default:
                return node;
        }
    }
    return node;
}