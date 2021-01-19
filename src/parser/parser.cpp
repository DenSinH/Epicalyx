#include "parser.h"
#include "expression/binop_expression.h"
#include "expression/cast_expression.h"
#include "expression/expression.h"
#include "expression/postfix_expression.h"
#include "expression/primary_expression.h"
#include "expression/unary_expression.h"

#include <stdexcept>


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
                    throw std::runtime_error("Invalid token: numerical constant of unknown type");
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
            throw std::runtime_error("Invalid syntax: expected primary expression, got keyword");
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
                auto right = ExpectUnaryExpression();
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

std::unique_ptr<Expr> Parser::ExpectUnaryExpression() {
    auto current = Current();
    switch(current->Type) {
        case TokenType::Incr: {
            EatType(TokenType::Incr);
            auto right = ExpectUnaryExpression();
            return std::make_unique<UnaryOpExpression>(UnaryOpExpression::UnOpType::PreIncrement, right);
        }
        case TokenType::Decr: {
            EatType(TokenType::Decr);
            auto right = ExpectUnaryExpression();
            return std::make_unique<UnaryOpExpression>(UnaryOpExpression::UnOpType::PreDecrement, right);
        }
        case TokenType::Ampersand: {
            EatType(TokenType::Ampersand);
            auto right = ExpectUnaryExpression();
            return std::make_unique<UnaryOpExpression>(UnaryOpExpression::UnOpType::Reference, right);
        }
        case TokenType::Asterisk: {
            EatType(TokenType::Asterisk);
            auto right = ExpectUnaryExpression();
            return std::make_unique<UnaryOpExpression>(UnaryOpExpression::UnOpType::Dereference, right);
        }
        case TokenType::Plus: {
            EatType(TokenType::Plus);
            auto right = ExpectUnaryExpression();
            return std::make_unique<UnaryOpExpression>(UnaryOpExpression::UnOpType::Positive, right);
        }
        case TokenType::Minus: {
            EatType(TokenType::Minus);
            auto right = ExpectUnaryExpression();
            return std::make_unique<UnaryOpExpression>(UnaryOpExpression::UnOpType::Negative, right);
        }
        case TokenType::Tilde: {
            EatType(TokenType::Tilde);
            auto right = ExpectUnaryExpression();
            return std::make_unique<UnaryOpExpression>(UnaryOpExpression::UnOpType::BinaryNot, right);
        }
        case TokenType::Exclamation: {
            EatType(TokenType::Exclamation);
            auto right = ExpectUnaryExpression();
            return std::make_unique<UnaryOpExpression>(UnaryOpExpression::UnOpType::LogicalNot, right);
        }
        case TokenType::Sizeof: {
            EatType(TokenType::Sizeof);
            // todo
            // check if next is paren and one after is type specifier/qualifier: sizeof type expression
            // otherwise, sizeof expression expression);
            log_fatal("Unimplemented: sizeof unary expression");
        }
        case TokenType::Alignof: {
            // todo
            log_fatal("Unimplemented: alignof unary expression");
        }
        default: {
            // normal postfix expression
            return ExpectPostfixExpression();
        }
    }
}

NODE(Expr) Parser::ExpectCastExpression() {
    if (Current()->Type == TokenType::LParen) {
        // check if type_name
        // todo
    }
    return ExpectUnaryExpression();
}

NODE(Expr) Parser::ExpectMultExpression() {
    return ExpectBinOpExpression<&Parser::ExpectCastExpression, TokenType::Asterisk, TokenType::Div, TokenType::Mod>();
}

NODE(Expr) Parser::ExpectAddExpression() {
    return ExpectBinOpExpression<&Parser::ExpectMultExpression, TokenType::Plus, TokenType::Minus>();
}

NODE(Expr) Parser::ExpectShiftExpression() {
    return ExpectBinOpExpression<&Parser::ExpectAddExpression, TokenType::LShift, TokenType::RShift>();
}

NODE(Expr) Parser::ExpectRelationalExpression() {
    return ExpectBinOpExpression<
            &Parser::ExpectShiftExpression,
            TokenType::LessEqual,
            TokenType::Less,
            TokenType::GreaterEqual,
            TokenType::Greater>();
}

NODE(Expr) Parser::ExpectEqualityExpression() {
    return ExpectBinOpExpression<&Parser::ExpectRelationalExpression, TokenType::Equal, TokenType::NotEqual>();
}

NODE(Expr) Parser::ExpectBinAndExpression() {
    return ExpectBinOpExpression<&Parser::ExpectEqualityExpression, TokenType::Ampersand>();
}

NODE(Expr) Parser::ExpectBinXorExpression() {
    return ExpectBinOpExpression<&Parser::ExpectBinAndExpression, TokenType::BinXor>();
}

NODE(Expr) Parser::ExpectBinOrExpression() {
    return ExpectBinOpExpression<&Parser::ExpectBinXorExpression, TokenType::BinOr>();
}

NODE(Expr) Parser::ExpectLogicAndExpression() {
    return ExpectBinOpExpression<&Parser::ExpectBinOrExpression, TokenType::LogicalAnd>();
}

NODE(Expr) Parser::ExpectLogicOrExpression() {
    return ExpectBinOpExpression<&Parser::ExpectLogicAndExpression, TokenType::LogicalOr>();
}
