#include "Parser.h"
#include "nodes/Expression.h"
#include "tokenizer/Token.h"
#include "ConstTokenVisitor.h"
#include "Is.h"


namespace epi {


pExpr Parser::EPrimary() {
  auto current = in_stream.Get();
  auto visitor = ConstTokenVisitor();
  switch (current->Class()) {
    case TokenClass::Identifier:
    case TokenClass::StringConstant:
    case TokenClass::NumericalConstant: {
      return current->GetConst(visitor);
    }
    case TokenClass::Punctuator: {
      // todo: type initializer?
      // has to be (expression)
      current->Expect(TokenType::LParen);
      auto expr = EExpression();
      in_stream.Eat(Token(TokenType::RParen));
      return expr;
    }
    default:
      throw calyx::FormatExcept("Unexpected token: got %s", current->to_string().c_str());
  }
}

pExpr Parser::EPostfix() {
  pToken current = in_stream.ForcePeek();
  if (current->type == TokenType::LParen) {
    // todo: check if typename
  }

  auto node = EPrimary();
  while (in_stream.Peek(current)) {
    switch (current->type) {
      case TokenType::LBracket: {
        // array access
        in_stream.Skip();
        auto right = EExpression();
        in_stream.Eat(Token(TokenType::RBracket));
        node = std::make_unique<ArrayAccess>(std::move(node), std::move(right));
        break;
      }
      case TokenType::LParen: {
        // function call
        auto func = std::make_unique<FunctionCall>(std::move(node));
        in_stream.Skip();
        while (in_stream.Peek(current) && current->type != TokenType::RParen) {
          func->AddArg(EAssignment());
        }
        in_stream.Eat(Token(TokenType::RParen));
        node = std::move(func);
        break;
      }
      case TokenType::Arrow:
        node = std::make_unique<Unary>(&CType::Deref, std::move(node));
        // fallthrough
      case TokenType::Dot: {
        in_stream.Skip();
        auto member = in_stream.Eat(Token(TokenType::Identifier));
        node = std::make_unique<MemberAccess>(std::move(node), std::dynamic_pointer_cast<tIdentifier>(member)->name);
        break;
      }
      case TokenType::Incr: {
        // expr++
        in_stream.Skip();
        node = std::make_unique<PostFix>(&CType::Incr, std::move(node));
        break;
      }
      case TokenType::Decr: {
        // expr--
        in_stream.Skip();
        node = std::make_unique<PostFix>(&CType::Decr, std::move(node));
        break;
      }
      default:
        return node;
    }
  }
  return node;
}

pExpr Parser::EUnary() {
  auto current = in_stream.ForcePeek();
  switch (current->type) {
    case TokenType::Incr: {
      // ++expr
      in_stream.Skip();
      auto right = EUnary();
      return std::make_unique<Unary>(&CType::Incr, std::move(right));
    }
    case TokenType::Decr: {
      // --expr
      in_stream.Skip();
      auto right = EUnary();
      return std::make_unique<Unary>(&CType::Decr, std::move(right));
    }
    case TokenType::Ampersand: {
      // &expr
      in_stream.Skip();
      auto right = EUnary();
      return std::make_unique<Unary>(&CType::Ref, std::move(right));
    }
    case TokenType::Asterisk: {
      // *expr
      in_stream.Skip();
      auto right = EUnary();
      return std::make_unique<Unary>(&CType::Deref, std::move(right));
    }
    case TokenType::Plus: {
      // +expr
      in_stream.Skip();
      auto right = EUnary();
      return std::make_unique<Unary>(&CType::Pos, std::move(right));
    }
    case TokenType::Minus: {
      // -expr
      in_stream.Skip();
      auto right = EUnary();
      return std::make_unique<Unary>(&CType::Neg, std::move(right));
    }
    case TokenType::Tilde: {
      // ~expr
      in_stream.Skip();
      auto right = EUnary();
      return std::make_unique<Unary>(&CType::BinNot, std::move(right));
    }
    case TokenType::Exclamation: {
      // !expr
      in_stream.Skip();
      auto right = EUnary();
      return std::make_unique<Unary>(&CType::LLogNot, std::move(right));
    }
    case TokenType::Sizeof: {
      // sizeof(expr) / sizeof(typename)
      throw std::runtime_error("unimplemented");
    }
    case TokenType::Alignof: {
      // _Alignof(expr)
      throw std::runtime_error("unimplemented");
    }
    default: {
      return EPostfix();
    }
  }
}

pExpr Parser::ECast() {
  auto current = in_stream.ForcePeek();
  if (current->type == TokenType::LParen) {
    // potential cast expression
  }
  return EUnary();
}

//template<pExpr (Parser::*SubNode)(), enum TokenType... types>
//pExpr Parser::EBinop() {
//  pExpr node = (this->*SubNode)();
//  while (!EndOfStream() && Is(Current()->Type).AnyOf<types...>()) {
//    auto current = Current();
//    Advance();
//    auto right = (this->*SubNode)();
//    node = MAKE_NODE(BinOpExpression)(
//            current,
//            BinOpExpression::TokenTypeToBinOp(current->Type),
//            std::move(node),
//            std::move(right)
//    );
//  }
//  return node;
//}

}