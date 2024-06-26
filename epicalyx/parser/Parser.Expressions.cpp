#include "Parser.h"

#include "TypeTraits.h"
#include "Stream.h"
#include "tokenizer/Token.h"
#include "ast/Expression.h"
#include "ast/Declaration.h"
#include "Decltype.h"


namespace epi {

using namespace ast;

pExpr ExprOrReduced(pExpr&& expr);

pExpr Parser::ResolveIdentifier(cotyl::CString&& name) const {
  // identifier might be enum value
  if (name.streq("__func__")) {
    if (!function_symbol) {
      throw ParserError("Use of reserved identifier '__func__' outside of function");
    }
    return std::make_unique<StringConstantNode>(cotyl::CString{function_symbol});
  }
  if (enum_values.Has(name)) {
    // replace enum values with constants immediately
    return std::make_unique<NumericalConstantNode<enum_type>>(enum_values.Get(name));
  }
  if (enum_values.Has(name)) {
    return std::make_unique<NumericalConstantNode<Parser::enum_type>>(enum_values.Get(name));
  }
  else if (variables.Has(name)) {
    return std::make_unique<IdentifierNode>(
      std::move(name),
      type::AnyType{variables.Get(name)}
    );
  }
  else {
    throw cotyl::FormatExceptStr<ParserError>("Undeclared identifier: '%s'", name);
  }
}

ast::pExpr Parser::EBinopBase() {
  // for the "regular" parser, the base for a binop expression is a cast expression
  return ECast();
}
  
pExpr Parser::EPostfix() {
  const Token* current;

  auto node = EPrimary();
  while (in_stream.Peek(current)) {
    switch (current->type) {
      case TokenType::LBracket: {
        // array access
        in_stream.Skip();
        auto right = EExpressionList();
        in_stream.Eat(TokenType::RBracket);
        node = std::make_unique<ArrayAccessNode>(std::move(node), std::move(right));

        // try to complete possible struct forward declarations
        CompleteForwardDecl(node->type);
        break;
      }
      case TokenType::LParen: {
        // function call
        cotyl::vector<pExpr> args{};
        in_stream.Skip();
        while (!in_stream.IsAfter(0, TokenType::RParen)) {
          args.push_back(EAssignment());
          if (!in_stream.IsAfter(0, TokenType::RParen)) {
            in_stream.Eat(TokenType::Comma);
          }
        }
        in_stream.Eat(TokenType::RParen);
        node = std::make_unique<FunctionCallNode>(std::move(node), std::move(args));
        break;
      }
      case TokenType::Arrow: {
        // indirect member access
        node = std::make_unique<UnopNode>(TokenType::Asterisk, std::move(node));

        // try to complete possible struct forward declarations
        CompleteForwardDecl(node->type);
      } [[fallthrough]];
      case TokenType::Dot: {
        // direct member access
        in_stream.Skip();
        auto member = in_stream.Eat(TokenType::Identifier);
        node = std::make_unique<MemberAccessNode>(std::move(node), std::move(member.get<IdentifierToken>().name));
        
        // try to complete possible struct forward declarations
        CompleteForwardDecl(node->type);
        break;
      }
      case TokenType::Incr: {
        // expr++
        in_stream.Skip();
        node = std::make_unique<PostFixNode>(TokenType::Incr, std::move(node));
        break;
      }
      case TokenType::Decr: {
        // expr--
        in_stream.Skip();
        node = std::make_unique<PostFixNode>(TokenType::Decr, std::move(node));
        break;
      }
      default:
        return ExprOrReduced(std::move(node));
    }
  }
  return ExprOrReduced(std::move(node));
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
      auto expr = std::make_unique<UnopNode>(type, std::move(right));
      
      if (type == TokenType::Asterisk) {
        // try to complete possible struct forward declarations
        CompleteForwardDecl(expr->type);
      }
      
      return ExprOrReduced(std::move(expr));
    }
    case TokenType::Sizeof: {
      // sizeof(expr) / sizeof(type-name)
      in_stream.Skip();
      if (in_stream.IsAfter(0, TokenType::LParen) && IsDeclarationSpecifier(1)) {
        in_stream.Eat(TokenType::LParen);
        auto type_name = ETypeName();
        in_stream.Eat(TokenType::RParen);
        return std::make_unique<NumericalConstantNode<u64>>(type_name->Sizeof());
      }
      return std::make_unique<NumericalConstantNode<u64>>(EUnary()->type->Sizeof());
    }
    case TokenType::Alignof: {
      // _Alignof(type-name)
      in_stream.EatSequence(TokenType::Alignof, TokenType::LParen);
      auto type_name = ETypeName();
      in_stream.Eat(TokenType::RParen);
      return std::make_unique<NumericalConstantNode<u64>>(type_name->Alignof());
    }
    default: {
      return EPostfix();
    }
  }
}

type::AnyType Parser::ETypeName() {
  auto ctype = DSpecifier();

  if (ctype.second != StorageClass::None) {
    throw ParserError("Storage class not allowed here");
  }

  auto decl = DDeclarator(ctype.first, StorageClass::None);
  if (!decl.name.empty()) {
    throw ParserError("Name not allowed in type name");
  }
  return std::move(decl.type);
}

pExpr Parser::ECast() {
  const Token* current = in_stream.ForcePeek();
  if (current->type == TokenType::LParen) {
    // potential cast expression or type initializer
    if (IsDeclarationSpecifier(1)) {
      in_stream.Skip();
      auto type_name = ETypeName();
      in_stream.Eat(TokenType::RParen);

      if (in_stream.IsAfter(0, TokenType::LBrace)) {
        // type initializer
        InitializerList list = EInitializerList();
        // this can never be reduced
        return std::make_unique<TypeInitializerNode>(std::move(type_name), std::move(list));
      }
      else {
        // regular cast expression
        auto expr = std::make_unique<CastNode>(std::move(type_name), ECast());
        return ExprOrReduced(std::move(expr));
      }
    }
  }
  return EUnary();
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
        // assignments can never be reduced
        return std::make_unique<AssignmentNode>(std::move(left), type, std::move(right));
      }
      default:
        break;
    }
  }
  return ExprOrReduced(std::move(left));
}

Initializer Parser::EInitializer() {
  if (in_stream.IsAfter(0, TokenType::LBrace)) {
    // initializer list
    auto value = EInitializerList();
    return {std::move(value)};
  }
  else {
    // assignment expression
    auto expr = EAssignment();
    return {std::move(ExprOrReduced(std::move(expr)))};
  }
}

InitializerList Parser::EInitializerList() {
  InitializerList list{};

  in_stream.Expect(TokenType::LBrace);
  while (!in_stream.IsAfter(0, TokenType::RBrace)) {
    if (in_stream.IsAfter(0, TokenType::Dot, TokenType::LBracket)) {  
      DesignatorList designator{};
      while (true) {
        // keep fetching nested declarators
        if (in_stream.EatIf(TokenType::Dot)) {
          // .member
          auto idt = in_stream.Expect(TokenType::Identifier);
          designator.emplace_back(std::move(idt.get<IdentifierToken>().name));
          if (in_stream.EatIf(TokenType::Assign)) {
            list.Push(std::move(designator), EInitializer());
            break;
          }
        }
        else {
          // [constant-expression]
          in_stream.Eat(TokenType::LBracket);
          designator.emplace_back(EConstexpr());
          in_stream.Eat(TokenType::RBracket);
          if (in_stream.EatIf(TokenType::Assign)) {
            list.Push(std::move(designator), EInitializer());
            break;
          }
        }
      }
    }
    else {
      list.Push({}, EInitializer());
    }

    if (!in_stream.EatIf(TokenType::Comma)) {
      // initializer list ends if no other comma is present
      break;
    }
  }
  in_stream.Expect(TokenType::RBrace);
  return std::move(list);
}

}