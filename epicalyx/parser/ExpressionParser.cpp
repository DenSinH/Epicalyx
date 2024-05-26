#include "ExpressionParser.h"

#include <memory>                 // for make_unique, unique_ptr

#include "Format.h"               // for FormatExceptStr
#include "SStream.h"              // for StringStream
#include "Stream.h"               // for Stream
#include "Variant.h"              // for Variant
#include "Vector.h"               // for vector
#include "ast/Expression.h"       // for CastNode, BinopNode, ExpressionList...
#include "ast/Node.h"             // for ExprNode
#include "swl/variant.hpp"        // for exhaustive
#include "tokenizer/Token.h"      // for AnyToken, KeywordToken (ptr only)
#include "tokenizer/TokenType.h"  // for TokenType
#include "types/AnyType.h"        // for AnyType, ValueType
#include "types/BaseType.h"       // for BaseType


namespace epi {

ExpressionParser::ExpressionParser(cotyl::Stream<AnyToken>& in_stream) : 
    in_stream{in_stream} { }


void ExpressionParser::PrintLoc(std::ostream& out) const {
  in_stream.PrintLoc(out);
}

using namespace ast;

pExpr ExprOrReduced(pExpr&& expr) {
  pExpr reduced = expr->type.visit<pExpr>(
    []<typename T>(const type::ValueType<T>& value) -> pExpr {
      if (value.value.has_value()) {
        return std::make_unique<NumericalConstantNode<T>>(value.value.value());
      }
      return nullptr;
    },
    [](const auto&) -> pExpr { return nullptr; }
  );
  if (reduced) return std::move(reduced);
  return std::move(expr);
}

pExpr ExpressionParser::EPrimary() {
  auto current = in_stream.Get();
  return current.visit<pExpr>(
    [&](IdentifierToken& ident) -> pExpr { 
      return ResolveIdentifier(std::move(ident.name));
    },
    [&](StringConstantToken& str) -> pExpr {
      cotyl::StringStream string{};
      string << str.value;
      while (in_stream.IsAfter(0, TokenType::StringConstant)) {
        string << in_stream.Get().get<StringConstantToken>().value;
      }
      return std::make_unique<StringConstantNode>(string.cfinalize());
    },
    [&](const PunctuatorToken& punc) -> pExpr {
      // has to be (expression)
      // type initializer caught in cast expression
      if (punc.type != TokenType::LParen) {
        throw cotyl::FormatExceptStr<ParserError>("Invalid token: expected (, got %s", punc);
      }
      
      auto expr = EExpressionList();
      in_stream.Eat(TokenType::RParen);
      return ExprOrReduced(std::move(expr));
    },
    [](const KeywordToken& keyw) -> pExpr {
      throw cotyl::FormatExceptStr<ParserError>("Unexpected token in primary expression: got %s", keyw);
    },
    []<typename T>(const NumericalConstantToken<T>& num) -> pExpr {
      return std::make_unique<NumericalConstantNode<T>>(num.value);
    },
    swl::exhaustive
  );
}

pExpr ExpressionParser::EBinopBase() {
  // cast expressions are not allowed in BaseParser expressions
  // we instead parse a unary expression with 
  // valid constexpr values
  auto type = in_stream.ForcePeek()->type;
  switch (type) {
    case TokenType::Plus:        // +expr
    case TokenType::Minus:       // -expr
    case TokenType::Tilde:       // ~expr
    case TokenType::Exclamation: // !expr
    {
      in_stream.Skip();
      auto right = EBinopBase();
      auto expr = std::make_unique<UnopNode>(type, std::move(right));
      return ExprOrReduced(std::move(expr));
    }
    case TokenType::Sizeof: {
      // sizeof(expr) / sizeof(type-name)
      in_stream.Skip();
      if (in_stream.IsAfter(0, TokenType::LParen) /* && IsDeclarationSpecifier(1) */) {
        throw cotyl::UnimplementedException("ExpressionParser typename");
        // in_stream.Eat(TokenType::LParen);
        // auto type_name = ETypeName();
        // in_stream.Eat(TokenType::RParen);
        // return std::make_unique<NumericalConstantNode<u64>>(type_name->Sizeof());
      }
      return std::make_unique<NumericalConstantNode<u64>>(ETernary()->type->Sizeof());
    }
    case TokenType::Alignof: {
      // _Alignof(type-name)
      in_stream.EatSequence(TokenType::Alignof, TokenType::LParen);
      throw cotyl::UnimplementedException("ExpressionParser typename");
      // auto type_name = ETypeName();
      // in_stream.Eat(TokenType::RParen);
      // return std::make_unique<NumericalConstantNode<u64>>(type_name->Alignof());
    }
    default: {
      return EPrimary();
    }
  }
}

template<pExpr (ExpressionParser::*SubNode)(), enum TokenType... types>
pExpr ExpressionParser::EBinopImpl() {
  pExpr node = (this->*SubNode)();
  const Token* current;
  while (in_stream.Peek(current) && ((current->type == types) || ...)) {
    auto type = in_stream.Get()->type;
    node = std::make_unique<BinopNode>(std::move(node), type, (this->*SubNode)());
  }
  return ExprOrReduced(std::move(node));
}

ast::pExpr ExpressionParser::EBinopBaseVCall() { 
  return EBinopBase(); 
};

constexpr auto EMul = &ExpressionParser::EBinopImpl<&ExpressionParser::EBinopBaseVCall, TokenType::Asterisk, TokenType::Div, TokenType::Mod>;
constexpr auto EAdd = &ExpressionParser::EBinopImpl<EMul, TokenType::Plus, TokenType::Minus>;
constexpr auto EShift = &ExpressionParser::EBinopImpl<EAdd, TokenType::LShift, TokenType::RShift>;
constexpr auto ERelational = &ExpressionParser::EBinopImpl<EShift, TokenType::Less, TokenType::Greater, TokenType::LessEqual, TokenType::GreaterEqual>;
constexpr auto EEquality = &ExpressionParser::EBinopImpl<ERelational, TokenType::Equal, TokenType::NotEqual>;
constexpr auto EAnd = &ExpressionParser::EBinopImpl<EEquality, TokenType::Ampersand>;
constexpr auto EOr = &ExpressionParser::EBinopImpl<EAnd, TokenType::BinOr>;
constexpr auto EXor = &ExpressionParser::EBinopImpl<EOr, TokenType::BinXor>;
constexpr auto ELogAnd = &ExpressionParser::EBinopImpl<EXor, TokenType::LogicalAnd>;
constexpr auto ELogOr = &ExpressionParser::EBinopImpl<ELogAnd, TokenType::LogicalOr>;

pExpr ExpressionParser::EBinop() {
  return (this->*ELogOr)();
}

pExpr ExpressionParser::ETernary() {
  auto left = EBinop();
  if (in_stream.IsAfter(0, TokenType::Question)) {
    in_stream.Skip();
    auto _true = EAssignment();
    in_stream.Eat(TokenType::Colon);
    auto _false = ETernary();

    // insert casts if true or false do not have the same type
    const auto& true_t = _true->type;
    const auto& false_t = _false->type;
    auto common_t = true_t.CommonType(false_t);

    if (!true_t.TypeEquals(common_t)) {
      _true = std::make_unique<CastNode>(type::AnyType(common_t), std::move(_true));
    }
    if (!false_t.TypeEquals(common_t)) {
      _false = std::make_unique<CastNode>(std::move(common_t), std::move(_false));
    }
    auto expr = std::make_unique<TernaryNode>(std::move(left), std::move(_true), std::move(_false));
    return ExprOrReduced(std::move(expr));
  }
  return ExprOrReduced(std::move(left));
}

pExpr ExpressionParser::EAssignment() {
  // Assignment expressions are not allowed in BaseParser expressions
  return ETernary();
}

pExpr ExpressionParser::EExpression() {
  // todo: commas
  auto expr = EAssignment();
  return ExprOrReduced(std::move(expr));
}

ast::pExpr ExpressionParser::EExpressionList() {
  cotyl::vector<pExpr> exprs{};
  do {
    exprs.emplace_back(ExprOrReduced(EExpression()));
  } while (in_stream.EatIf(TokenType::Comma));
  
  // expressions size is at least 1, since we use a do/while loop
  if (exprs.size() == 1) {
    return std::move(exprs[0]);
  }
  else {
    return std::make_unique<ExpressionListNode>(std::move(exprs));
  }
}

i64 ExpressionParser::EConstexpr() {
  auto expr = EExpression();
  return expr->ConstIntVal();
}

}
