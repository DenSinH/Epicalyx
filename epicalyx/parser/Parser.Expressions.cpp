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
      // todo: is (expression), but the list part makes no sense
      auto expr = EExpression();
      in_stream.Eat(TokenType::RParen);
      return expr;
    }
    default:
      throw cotyl::FormatExceptStr("Unexpected token: got %s", current->to_string());
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
        // todo: is EExpression, but the commas make no sense
        auto right = EExpression();
        in_stream.Eat(TokenType::RBracket);
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
        in_stream.Eat(TokenType::RParen);
        node = std::move(func);
        break;
      }
      case TokenType::Arrow:
        node = std::make_unique<Unary>(TokenType::Asterisk, std::move(node));
        // fallthrough
      case TokenType::Dot: {
        in_stream.Skip();
        auto member = in_stream.Eat(TokenType::Identifier);
        node = std::make_unique<MemberAccess>(std::move(node), std::dynamic_pointer_cast<tIdentifier>(member)->name);
        break;
      }
      case TokenType::Incr: {
        // expr++
        in_stream.Skip();
        node = std::make_unique<PostFix>(TokenType::Incr, std::move(node));
        break;
      }
      case TokenType::Decr: {
        // expr--
        in_stream.Skip();
        node = std::make_unique<PostFix>(TokenType::Decr, std::move(node));
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
    case TokenType::Incr: // ++expr
    case TokenType::Decr: // --expr
    case TokenType::Ampersand: // &expr
    case TokenType::Asterisk: // *expr
    case TokenType::Plus: // +expr
    case TokenType::Minus: // -expr
    case TokenType::Tilde: // ~expr
    case TokenType::Exclamation: // !expr
    {
      in_stream.Skip();
      auto right = EUnary();
      return std::make_unique<Unary>(current->type, std::move(right));
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

template<pExpr (Parser::*SubNode)(), enum TokenType... types>
pExpr Parser::EBinopImpl() {
  pExpr node = (this->*SubNode)();
  pToken current;
  while (in_stream.Peek(current) && cotyl::Is(current->type).AnyOf<types...>()) {
    in_stream.Skip();
    node = std::make_unique<Binop>(std::move(node), current->type, (this->*SubNode)());
  }
  return node;
}

constexpr auto EMul = &Parser::EBinopImpl<&Parser::ECast, TokenType::Asterisk, TokenType::Div, TokenType::Mod>;
constexpr auto EAdd = &Parser::EBinopImpl<EMul, TokenType::Plus, TokenType::Minus>;
constexpr auto EShift = &Parser::EBinopImpl<EAdd, TokenType::LShift, TokenType::RShift>;
constexpr auto ERelational = &Parser::EBinopImpl<EShift, TokenType::Less, TokenType::Greater, TokenType::LessEqual, TokenType::GreaterEqual>;
constexpr auto EEquality = &Parser::EBinopImpl<ERelational, TokenType::Equal, TokenType::NotEqual>;
constexpr auto EAnd = &Parser::EBinopImpl<EEquality, TokenType::Ampersand>;
constexpr auto EOr = &Parser::EBinopImpl<EAnd, TokenType::BinOr>;
constexpr auto EXor = &Parser::EBinopImpl<EOr, TokenType::BinXor>;
constexpr auto ELogAnd = &Parser::EBinopImpl<EXor, TokenType::LogicalAnd>;
constexpr auto ELogOr = &Parser::EBinopImpl<ELogAnd, TokenType::LogicalOr>;

pExpr Parser::EBinop() {
  return (this->*ELogOr)();
}

pExpr Parser::ETernary() {
  auto left = EBinop();
  if (in_stream.IsAfter(0, TokenType::Question)) {
    in_stream.Skip();
    auto _true = EAssignment();
    in_stream.Eat(TokenType::Colon);
    auto _false = ETernary();
    return std::make_unique<Ternary>(std::move(left), std::move(_true), std::move(_false));
  }
  return left;
}

pExpr Parser::EAssignment() {
  // for the left hand side this has to actually be a conditional expression,
  // but if it's a proper assignment, that will happen regardless
  auto left = ETernary();
  pToken current;
  if (in_stream.Peek(current)) {
    switch (current->type) {
      case TokenType::Assign:
      case TokenType::IMul:
      case TokenType::IDiv:
      case TokenType::IMod:
      case TokenType::IPlus:
      case TokenType::IMinus:
      case TokenType::ILShift:
      case TokenType::IRShift:
      case TokenType::IAnd:
      case TokenType::IOr:
      case TokenType::IXor: {
        in_stream.Skip();
        auto right = EAssignment();
        return std::make_unique<Assignment>(std::move(left), current->type, std::move(right));
      }
      default:
        break;
    }
  }
  return left;
}

pExpr Parser::EExpression() {
  // todo: commas
  return EAssignment();
}

void Parser::EExpressionList(std::vector<pExpr>& dest) {
  do {
    dest.push_back(EExpression());
  } while (in_stream.EatIf(TokenType::Comma));
}

}