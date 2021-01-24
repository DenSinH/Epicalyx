#include "parser.h"
#include "expression_nodes.h"

#include <stdexcept>

NODE(ExprNode) Parser::ExpectPrimaryExpression() {
    auto current = Current();
    switch (current->Class) {
        case TokenClass::Identifier: {
            Advance();
            return MAKE_NODE(PrimaryExpressionIdentifier)(std::static_pointer_cast<Identifier>(current)->Name);
        }
        case TokenClass::NumericalConstant: {
            Advance();
            switch(current->Type) {
                case TokenType::ConstInt:
                    return MAKE_NODE(PrimaryExpressionConstant<int>)(std::static_pointer_cast<NumericalConstant<int>>(current)->Value);
                case TokenType::ConstUnsignedInt:
                    return MAKE_NODE(PrimaryExpressionConstant<unsigned int>)(std::static_pointer_cast<NumericalConstant<unsigned int>>(current)->Value);
                case TokenType::ConstLong:
                    return MAKE_NODE(PrimaryExpressionConstant<long>)(std::static_pointer_cast<NumericalConstant<long>>(current)->Value);
                case TokenType::ConstUnsignedLong:
                    return MAKE_NODE(PrimaryExpressionConstant<unsigned long>)(std::static_pointer_cast<NumericalConstant<unsigned long>>(current)->Value);
                case TokenType::ConstLongLong:
                    return MAKE_NODE(PrimaryExpressionConstant<long long>)(std::static_pointer_cast<NumericalConstant<long long>>(current)->Value);
                case TokenType::ConstUnsignedLongLong:
                    return MAKE_NODE(PrimaryExpressionConstant<unsigned long long>)(std::static_pointer_cast<NumericalConstant<unsigned long long>>(current)->Value);
                case TokenType::ConstFloat:
                    return MAKE_NODE(PrimaryExpressionConstant<float>)(std::static_pointer_cast<NumericalConstant<float>>(current)->Value);
                case TokenType::ConstDouble:
                    return MAKE_NODE(PrimaryExpressionConstant<double>)(std::static_pointer_cast<NumericalConstant<double>>(current)->Value);
                default:
                    throw std::runtime_error("Invalid token: numerical constant of unknown type");
            }
        }
        case TokenClass::StringConstant: {
            Advance();
            return MAKE_NODE(PrimaryStringLiteral)(std::static_pointer_cast<StringConstant>(current)->Value);
        }
        case TokenClass::Punctuator: {
            // has to be ( expression )
            EatType(TokenType::LParen);
            auto expr = ExpectExpression();
            EatType(TokenType::RParen);
            return expr;
        }
        default:
            throw std::runtime_error("Invalid syntax: expected primary expression, got keyword");
    }
}

std::unique_ptr<ExprNode> Parser::ExpectPostfixExpression() {
    auto current = Current();
    if (current->Type == TokenType::LParen) {
        if (IsTypeName(1)) {
            return ExpectCastExpressionOrTypeInitializer();
        }
        return ExpectPrimaryExpression();
    }

    auto node = ExpectPrimaryExpression();
    while (!EndOfStream()) {
        current = Current();
        switch(current->Type) {
            case TokenType::LBracket: {
                // array access
                EatType(TokenType::LBracket);
                auto right = ExpectExpression();
                EatType(TokenType::RBracket);
                node = MAKE_NODE(ArrayAccessExpression)(node, right);
                break;
            }
            case TokenType::LParen: {
                // function call
                EatType(TokenType::LParen);
                if (Current()->Type == TokenType::RParen) {
                    EatType(TokenType::RParen);
                    return MAKE_NODE(FunctionCallExpression)(node);
                }
                auto args = ExpectArgumentListExpression();
                EatType(TokenType::RParen);
                return MAKE_NODE(FunctionCallExpression)(node, args);
            }
            case TokenType::Dot:
            case TokenType::Arrow: {
                // member access
                auto type = (current->Type == TokenType::Dot) ?
                        MemberAccessExpression::MemberAccessType::Direct :
                        MemberAccessExpression::MemberAccessType::Pointer;
                Advance();
                auto field = EatType(TokenType::Identifier);
                node = MAKE_NODE(MemberAccessExpression)(node, std::static_pointer_cast<Identifier>(field)->Name, type);
                break;
            }
            case TokenType::Incr:
            case TokenType::Decr: {
                // ++ / --
                auto type = (current->Type == TokenType::Incr) ?
                        PostCrementExpression::CrementType::Increment :
                        PostCrementExpression::CrementType::Decrement;
                Advance();
                node = MAKE_NODE(PostCrementExpression)(node, type);
                break;
            }
            default:
                return node;
        }
    }
    return node;
}

std::unique_ptr<ExprNode> Parser::ExpectUnaryExpression() {
    auto current = Current();
    switch(current->Type) {
        case TokenType::Incr: {
            EatType(TokenType::Incr);
            auto right = ExpectUnaryExpression();
            return MAKE_NODE(UnaryOpExpression)(UnaryOpExpression::UnOpType::PreIncrement, right);
        }
        case TokenType::Decr: {
            EatType(TokenType::Decr);
            auto right = ExpectUnaryExpression();
            return MAKE_NODE(UnaryOpExpression)(UnaryOpExpression::UnOpType::PreDecrement, right);
        }
        case TokenType::Ampersand: {
            EatType(TokenType::Ampersand);
            auto right = ExpectUnaryExpression();
            return MAKE_NODE(UnaryOpExpression)(UnaryOpExpression::UnOpType::Reference, right);
        }
        case TokenType::Asterisk: {
            EatType(TokenType::Asterisk);
            auto right = ExpectUnaryExpression();
            return MAKE_NODE(UnaryOpExpression)(UnaryOpExpression::UnOpType::Dereference, right);
        }
        case TokenType::Plus: {
            EatType(TokenType::Plus);
            auto right = ExpectUnaryExpression();
            return MAKE_NODE(UnaryOpExpression)(UnaryOpExpression::UnOpType::Positive, right);
        }
        case TokenType::Minus: {
            EatType(TokenType::Minus);
            auto right = ExpectUnaryExpression();
            return MAKE_NODE(UnaryOpExpression)(UnaryOpExpression::UnOpType::Negative, right);
        }
        case TokenType::Tilde: {
            EatType(TokenType::Tilde);
            auto right = ExpectUnaryExpression();
            return MAKE_NODE(UnaryOpExpression)(UnaryOpExpression::UnOpType::BinaryNot, right);
        }
        case TokenType::Exclamation: {
            EatType(TokenType::Exclamation);
            auto right = ExpectUnaryExpression();
            return MAKE_NODE(UnaryOpExpression)(UnaryOpExpression::UnOpType::LogicalNot, right);
        }
        case TokenType::Sizeof: {
            Advance();
            if (Current()->Type == TokenType::LParen) {
                // check if type_name
                if (IsTypeName(1)) {
                    Advance();
                    auto type_name = ExpectTypeName();
                    EatType(TokenType::RParen);
                    return MAKE_NODE(SizeOfTypeExpression)(type_name);
                }
                auto expr = ExpectUnaryExpression();
                return MAKE_NODE(SizeOfExpression)(expr);
            }
        }
        case TokenType::Alignof: {
            Advance();
            EatType(TokenType::LParen);
            auto type_name = ExpectTypeName();
            EatType(TokenType::RParen);
            return MAKE_NODE(AlignOfExpression)(type_name);
        }
        default: {
            // normal postfix expression
            return ExpectPostfixExpression();
        }
    }
}

NODE(ExprNode) Parser::ExpectCastExpressionOrTypeInitializer() {
    if (Current()->Type == TokenType::LParen) {
        // check if type_name
        if (IsTypeName(1)) {
            Advance();
            auto type_name = ExpectTypeName();
            EatType(TokenType::RParen);
            if (!EndOfStream() && Current()->Type == TokenType::LBrace) {
                // type initializer expression
                auto initializer_list = ExpectInitializerList();
                return MAKE_NODE(TypeInitializerExpression)(type_name, initializer_list);
            }
            auto expr = ExpectCastExpressionOrTypeInitializer();
            return MAKE_NODE(CastExpression)(type_name, expr);
        }
    }
    return ExpectUnaryExpression();
}

NODE(ExprNode) Parser::ExpectMultExpression() {
    return ExpectBinOpExpression<&Parser::ExpectCastExpressionOrTypeInitializer, TokenType::Asterisk, TokenType::Div, TokenType::Mod>();
}

NODE(ExprNode) Parser::ExpectAddExpression() {
    return ExpectBinOpExpression<&Parser::ExpectMultExpression, TokenType::Plus, TokenType::Minus>();
}

NODE(ExprNode) Parser::ExpectShiftExpression() {
    return ExpectBinOpExpression<&Parser::ExpectAddExpression, TokenType::LShift, TokenType::RShift>();
}

NODE(ExprNode) Parser::ExpectRelationalExpression() {
    return ExpectBinOpExpression<
            &Parser::ExpectShiftExpression,
            TokenType::LessEqual,
            TokenType::Less,
            TokenType::GreaterEqual,
            TokenType::Greater>();
}

NODE(ExprNode) Parser::ExpectEqualityExpression() {
    return ExpectBinOpExpression<&Parser::ExpectRelationalExpression, TokenType::Equal, TokenType::NotEqual>();
}

NODE(ExprNode) Parser::ExpectBinAndExpression() {
    return ExpectBinOpExpression<&Parser::ExpectEqualityExpression, TokenType::Ampersand>();
}

NODE(ExprNode) Parser::ExpectBinXorExpression() {
    return ExpectBinOpExpression<&Parser::ExpectBinAndExpression, TokenType::BinXor>();
}

NODE(ExprNode) Parser::ExpectBinOrExpression() {
    return ExpectBinOpExpression<&Parser::ExpectBinXorExpression, TokenType::BinOr>();
}

NODE(ExprNode) Parser::ExpectLogicAndExpression() {
    return ExpectBinOpExpression<&Parser::ExpectBinOrExpression, TokenType::LogicalAnd>();
}

NODE(ExprNode) Parser::ExpectLogicOrExpression() {
    return ExpectBinOpExpression<&Parser::ExpectLogicAndExpression, TokenType::LogicalOr>();
}

NODE(ExprNode) Parser::ExpectConditionalExpression() {
    auto node = ExpectLogicOrExpression();
    if (!EndOfStream() && Current()->Type == TokenType::Question) {
        // actual conditional expression
        // unary-expression ? expression : conditional-expression
        EatType(TokenType::Question);
        auto t = ExpectExpression();
        EatType(TokenType::Colon);
        auto f = ExpectConditionalExpression();
        node = MAKE_NODE(CondExpression)(node, t, f);
    }
    return node;
}

NODE(ExprNode) Parser::ExpectAssignmentExpression() {
    auto node = ExpectConditionalExpression();

    // if node is of type UnaryExpression and
    if (!EndOfStream()) {
        auto current = Current()->Type;
        if (Is(current).AnyOf<
            TokenType::Assign,
            TokenType::IMul,
            TokenType::IDiv,
            TokenType::IMod,
            TokenType::IPlus,
            TokenType::IMinus,
            TokenType::ILShift,
            TokenType::IRShift,
            TokenType::IAnd,
            TokenType::IXor,
            TokenType::IOr
        >()) {
            // assignment expression
            Advance();
            auto value = ExpectAssignmentExpression();
            node = MAKE_NODE(AssignmentExpression)(node, AssignmentExpression::TokenTypeToAssignOp(current), value);
        }
    }
    return node;
}

NODE(ExprNode) Parser::ExpectExpression() {
    auto ctx = context("parsing expression");

    Dump();

    auto node = ExpectAssignmentExpression();
    while (!EndOfStream() && Current()->Type == TokenType::Comma) {
        EatType(TokenType::Comma);
        auto next = ExpectAssignmentExpression();
        node = MAKE_NODE(Expression)(node, next);
    }
    return node;
}

NODE(ExprNode) Parser::ExpectArgumentListExpression() {
    // same grammar:
    return ExpectExpression();
}