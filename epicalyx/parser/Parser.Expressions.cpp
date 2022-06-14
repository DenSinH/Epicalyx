#include "Parser.h"
#include "ast/Expression.h"
#include "ast/Declaration.h"
#include "tokenizer/Token.h"
#include "ConstTokenVisitor.h"
#include "Is.h"
#include "Cast.h"


namespace epi {

using namespace ast;

pExpr Parser::EPrimary() {
  auto current = in_stream.Get();
  auto visitor = ConstTokenVisitor();
  switch (current->Class()) {
    case TokenClass::Identifier: {
      // identifier might be enum value
      std::string name = cotyl::unique_ptr_cast<tIdentifier>(current)->name;
      if (enum_values.Has(name)) {
        // replace enum values with constants immediately
        return std::make_unique<NumericalConstant<enum_type>>(enum_values.Get(name));
      }
    } [[fallthrough]];
    case TokenClass::StringConstant:
    case TokenClass::NumericalConstant: {
      return current->GetConst(visitor);
    }
    case TokenClass::Punctuator: {
      // has to be (expression)
      // type initializer caught in cast expression
      current->Expect(TokenType::LParen);
      // todo: is (expression), but the list part makes no sense
      auto expr = EExpression();
      in_stream.Eat(TokenType::RParen);
      return expr;
    }
    default:
      throw cotyl::FormatExceptStr("Unexpected token in primary expression: got %s", current);
  }
}

pExpr Parser::EPostfix() {
  const Token* current;

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
      case TokenType::Arrow: {
        // indirect member access
        in_stream.Skip();
        auto member = in_stream.Eat(TokenType::Identifier);
        node = std::make_unique<MemberAccess>(std::move(node), false, cotyl::unique_ptr_cast<tIdentifier>(member)->name);
        break;
      }
      case TokenType::Dot: {
        // direct member access
        in_stream.Skip();
        auto member = in_stream.Eat(TokenType::Identifier);
        node = std::make_unique<MemberAccess>(std::move(node), true, cotyl::unique_ptr_cast<tIdentifier>(member)->name);
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
  auto type = in_stream.ForcePeek()->type;
  switch (type) {
    case TokenType::Incr:        // ++expr
    case TokenType::Decr:        // --expr
    case TokenType::Ampersand:   // &expr
    case TokenType::Asterisk:    // *expr
    case TokenType::Plus:        // +expr
    case TokenType::Minus:       // -expr
    case TokenType::Tilde:       // ~expr
    case TokenType::Exclamation: // !expr
    {
      in_stream.Skip();
      auto right = ECast();
      return std::make_unique<Unary>(type, std::move(right));
    }
    case TokenType::Sizeof: {
      // sizeof(expr) / sizeof(type-name)
      in_stream.Skip();
      if (in_stream.IsAfter(0, TokenType::LParen) && IsDeclarationSpecifier(1)) {
        in_stream.Eat(TokenType::LParen);
        auto type_name = ETypeName();
        in_stream.Eat(TokenType::RParen);
        return std::make_unique<NumericalConstant<u64>>(type_name->Sizeof());
      }
      return std::make_unique<NumericalConstant<u64>>(EExpression()->SemanticAnalysis(*this)->Sizeof());
    }
    case TokenType::Alignof: {
      // _Alignof(type-name)
      in_stream.EatSequence(TokenType::Alignof, TokenType::LParen);
      auto type_name = ETypeName();
      in_stream.Eat(TokenType::RParen);
      return std::make_unique<NumericalConstant<u64>>(type_name->Alignof());
    }
    default: {
      return EPostfix();
    }
  }
}

pType<const CType> Parser::ETypeName() {
  auto ctype = DSpecifier();

  if (ctype.second != StorageClass::None) {
    throw std::runtime_error("Storage class not allowed here");
  }

  pNode<Declaration> decl = DDeclarator(ctype.first, StorageClass::None);
  if (!decl->name.empty()) {
    throw std::runtime_error("Name not allowed in type name");
  }
  return decl->type;
}

pExpr Parser::ECast() {
  const Token* current = in_stream.ForcePeek();
  if (current->type == TokenType::LParen) {
    // potential cast expression or type initializer
    if (IsDeclarationSpecifier(1)) {
      in_stream.Skip();
      auto type_name = ETypeName();
      in_stream.Eat(TokenType::RParen);

      if (in_stream.EatIf(TokenType::LBrace)) {
        // type initializer
        pNode<InitializerList> list = EInitializerList();
        in_stream.Eat(TokenType::RBrace);
        return std::make_unique<TypeInitializer>(type_name, std::move(list));
      }
      else {
        // regular cast expression
        return std::make_unique<Cast>(type_name, ECast());
      }
    }
  }
  return EUnary();
}

template<pExpr (Parser::*SubNode)(), enum TokenType... types>
pExpr Parser::EBinopImpl() {
  pExpr node = (this->*SubNode)();
  const Token* current;
  while (in_stream.Peek(current) && cotyl::Is(current->type).AnyOf<types...>()) {
    auto type = in_stream.Get()->type;
    node = std::make_unique<Binop>(std::move(node), type, (this->*SubNode)());
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

    // insert casts if true or false do not have the same type
    auto true_t = _true->SemanticAnalysis(*this);
    auto false_t = _false->SemanticAnalysis(*this);
    auto common_t = true_t->CommonType(*false_t);

    if (!true_t->EqualType(*common_t)) {
      _true = std::make_unique<Cast>(common_t, std::move(_true));
    }
    if (!false_t->EqualType(*common_t)) {
      _false = std::make_unique<Cast>(common_t, std::move(_false));
    }
    return std::make_unique<Ternary>(std::move(left), std::move(_true), std::move(_false));
  }
  return left;
}

pExpr Parser::EAssignment() {
  // for the left hand side this has to actually be a conditional expression,
  // but if it's a proper assignment, that will happen regardless
  auto left = ETernary();
  const Token* current;
  if (in_stream.Peek(current)) {
    auto type = current->type;
    switch (type) {
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
        return std::make_unique<Assignment>(std::move(left), type, std::move(right));
      }
      default:
        break;
    }
  }
  return left;
}

pExpr Parser::EExpression() {
  // todo: commas
  auto expr = EAssignment();
  expr->SemanticAnalysis(*this);
  return std::move(expr);
}

i64 Parser::EConstexpr() {
  auto expr = ETernary();
  expr->SemanticAnalysis(*this);
  return expr->ConstEval();
}

void Parser::EExpressionList(std::vector<pExpr>& dest) {
  do {
    dest.push_back(EExpression());
  } while (in_stream.EatIf(TokenType::Comma));
}

Initializer Parser::EInitializer() {
  if (in_stream.EatIf(TokenType::LBrace)) {
    // initializer list
    auto value = EInitializerList();
    in_stream.Eat(TokenType::RBrace);
    return value;
  }
  else {
    // assignment expression
    auto expr = EAssignment();
    expr->SemanticAnalysis(*this);
    return std::move(expr);
  }
}

pNode<InitializerList> Parser::EInitializerList() {
  pNode<InitializerList> list = std::make_unique<InitializerList>();

  while (!in_stream.IsAfter(0, TokenType::RBrace)) {
    DesignatorList designator{};
    if (in_stream.IsAfter(0, TokenType::Dot, TokenType::LBracket)) {
      while (true) {
        // keep fetching nested declarators
        if (in_stream.EatIf(TokenType::Dot)) {
          // .member
          in_stream.Expect(TokenType::Identifier);
          designator.emplace_back(cotyl::unique_ptr_cast<tIdentifier>(in_stream.Get())->name);
          if (in_stream.EatIf(TokenType::Assign)) {
            list->Push(std::move(designator), EInitializer());
            break;
          }
        }
        else {
          // [constant-expression]
          in_stream.Skip();
          designator.emplace_back(EConstexpr());
          in_stream.Eat(TokenType::RBracket);
          if (in_stream.EatIf(TokenType::Assign)) {
            list->Push(std::move(designator), EInitializer());
            break;
          }
        }
      }
    }
    else {
      list->Push({}, EInitializer());
    }

    if (!in_stream.EatIf(TokenType::Comma)) {
      // initializer list ends if no other comma is present
      in_stream.Expect(TokenType::RBrace);
    }
  }
  return list;
}

}