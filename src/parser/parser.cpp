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

std::unique_ptr<Node> Parser::ExpectPrimaryExpression() {
    auto current = Current();
    switch (current->Class) {
        case TokenClass::Identifier: {
            return std::make_unique<PrimaryExpressionIdentifier>(std::static_pointer_cast<Identifier>(current)->Name);
        }
        case TokenClass::NumericalConstant: {
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
            return std::make_unique<PrimaryStringLiteral>(std::static_pointer_cast<StringConstant>(current)->Value);
        }
        case TokenClass::Punctuator: {
            // has to be ( expression )
            ExpectType(TokenType::LParen);
            // todo: expression
        }
        default:
            log_fatal("Invalid syntax: expected primary expression, got keyword");
    }
}