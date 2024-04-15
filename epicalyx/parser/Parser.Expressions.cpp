#include "Parser.h"

#include "TypeTraits.h"
#include "Stream.h"
#include "tokenizer/Token.h"
#include "ast/Expression.h"
#include "ast/Declaration.h"


namespace epi {

using namespace ast;

pExpr ConstParser::EPrimary() {
  auto current = in_stream.Get();
  return current.visit<pExpr>(
    [](const IdentifierToken& ident) -> pExpr { 
      throw cotyl::UnexpectedIdentifierException();
    },
    [](const StringConstantToken& str) -> pExpr {
      return std::make_unique<StringConstantNode>(str.value);
    },
    [&](const PunctuatorToken& punc) -> pExpr {
      // has to be (ternary), since in the BaseParser we do not expect assignment
      // type initializer caught in cast expression
      if (punc.type != TokenType::LParen) {
        throw cotyl::FormatExceptStr("Invalid token: expected (, got %s", current);
      }
      auto expr = ETernary();
      in_stream.Eat(TokenType::RParen);
      return expr;
    },
    [](const KeywordToken& keyw) -> pExpr {
      throw cotyl::FormatExceptStr("Unexpected token in primary expression: got %s", keyw);
    },
    [](const auto& num) -> pExpr {
      using tok_t = std::decay_t<decltype(num)>;
      static_assert(cotyl::is_instantiation_of_v<NumericalConstantToken, tok_t>);
      auto val = num.value;
      return std::make_unique<NumericalConstantNode<decltype(val)>>(val);
    }
  );
}

pExpr Parser::EPrimary() {
  auto current = in_stream.Get();
  return current.visit<pExpr>(
    [&](const IdentifierToken& ident) -> pExpr { 
      // identifier might be enum value
      std::string name = ident.name;
      if (enum_values.Has(name)) {
        // replace enum values with constants immediately
        return std::make_unique<NumericalConstantNode<enum_type>>(enum_values.Get(name));
      }
      return std::make_unique<IdentifierNode>(ident.name);
    },
    [](const StringConstantToken& str) -> pExpr {
      return std::make_unique<StringConstantNode>(str.value);
    },
    [&](const PunctuatorToken& punc) -> pExpr {
      // has to be (expression)
      // type initializer caught in cast expression
      if (punc.type != TokenType::LParen) {
        throw cotyl::FormatExceptStr("Invalid token: expected (, got %s", punc);
      }
      // todo: is (expression), but the list part makes no sense
      auto expr = EExpression();
      in_stream.Eat(TokenType::RParen);
      return expr;
    },
    [](const KeywordToken& keyw) -> pExpr {
      throw cotyl::FormatExceptStr("Unexpected token in primary expression: got %s", keyw);
    },
    [](const auto& num) -> pExpr {
      using tok_t = std::decay_t<decltype(num)>;
      static_assert(cotyl::is_instantiation_of_v<NumericalConstantToken, tok_t>);
      auto val = num.value;
      return std::make_unique<NumericalConstantNode<decltype(val)>>(val);
    }
  );
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
        node = std::make_unique<ArrayAccessNode>(std::move(node), std::move(right));
        break;
      }
      case TokenType::LParen: {
        // function call
        auto func = std::make_unique<FunctionCallNode>(std::move(node));
        in_stream.Skip();
        while (!in_stream.IsAfter(0, TokenType::RParen)) {
          func->AddArg(EAssignment());
          if (!in_stream.IsAfter(0, TokenType::RParen)) {
            in_stream.Eat(TokenType::Comma);
          }
        }
        in_stream.Eat(TokenType::RParen);
        node = std::move(func);
        break;
      }
      case TokenType::Arrow: {
        // indirect member access
        in_stream.Skip();
        auto member = in_stream.Eat(TokenType::Identifier);
        node = std::make_unique<MemberAccessNode>(std::move(node), false, member.get<IdentifierToken>().name);
        break;
      }
      case TokenType::Dot: {
        // direct member access
        in_stream.Skip();
        auto member = in_stream.Eat(TokenType::Identifier);
        node = std::make_unique<MemberAccessNode>(std::move(node), true, member.get<IdentifierToken>().name);
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
      return std::make_unique<UnopNode>(type, std::move(right));
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
      return std::make_unique<NumericalConstantNode<u64>>(EExpression()->SemanticAnalysis(*this)->Sizeof());
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

pType<const CType> Parser::ETypeName() {
  auto ctype = DSpecifier();

  if (ctype.second != StorageClass::None) {
    throw std::runtime_error("Storage class not allowed here");
  }

  auto decl = DDeclarator(ctype.first, StorageClass::None);
  if (!decl->name.empty()) {
    throw std::runtime_error("Name not allowed in type name");
  }
  return decl->type;
}

pExpr ConstParser::ECast() {
  // cast expressions are not allowed in BaseParser expressions
  return this->ConstParser::EPrimary();
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
        return std::make_unique<TypeInitializerNode>(type_name, std::move(list));
      }
      else {
        // regular cast expression
        return std::make_unique<CastNode>(type_name, ECast());
      }
    }
  }
  return EUnary();
}

template<pExpr (ConstParser::*SubNode)(), enum TokenType... types>
pExpr ConstParser::EBinopImpl() {
  pExpr node = (this->*SubNode)();
  const Token* current;
  while (in_stream.Peek(current) && ((current->type == types) || ...)) {
    auto type = in_stream.Get()->type;
    node = std::make_unique<BinopNode>(std::move(node), type, (this->*SubNode)());
  }
  return node;
}

constexpr auto EMul = &ConstParser::EBinopImpl<&ConstParser::ECast, TokenType::Asterisk, TokenType::Div, TokenType::Mod>;
constexpr auto EAdd = &ConstParser::EBinopImpl<EMul, TokenType::Plus, TokenType::Minus>;
constexpr auto EShift = &ConstParser::EBinopImpl<EAdd, TokenType::LShift, TokenType::RShift>;
constexpr auto ERelational = &ConstParser::EBinopImpl<EShift, TokenType::Less, TokenType::Greater, TokenType::LessEqual, TokenType::GreaterEqual>;
constexpr auto EEquality = &ConstParser::EBinopImpl<ERelational, TokenType::Equal, TokenType::NotEqual>;
constexpr auto EAnd = &ConstParser::EBinopImpl<EEquality, TokenType::Ampersand>;
constexpr auto EOr = &ConstParser::EBinopImpl<EAnd, TokenType::BinOr>;
constexpr auto EXor = &ConstParser::EBinopImpl<EOr, TokenType::BinXor>;
constexpr auto ELogAnd = &ConstParser::EBinopImpl<EXor, TokenType::LogicalAnd>;
constexpr auto ELogOr = &ConstParser::EBinopImpl<ELogAnd, TokenType::LogicalOr>;

pExpr ConstParser::EBinop() {
  return (this->*ELogOr)();
}

pExpr ConstParser::ETernary() {
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
      _true = std::make_unique<CastNode>(common_t, std::move(_true));
    }
    if (!false_t->EqualType(*common_t)) {
      _false = std::make_unique<CastNode>(common_t, std::move(_false));
    }
    return std::make_unique<TernaryNode>(std::move(left), std::move(_true), std::move(_false));
  }
  return left;
}

pExpr ConstParser::EAssignment() {
  // Assignment expressions are not allowed in BaseParser expressions
  return ETernary();
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
        return std::make_unique<AssignmentNode>(std::move(left), type, std::move(right));
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

i64 ConstParser::EConstexpr() {
  auto expr = ETernary();
  expr->SemanticAnalysis(*this);
  return expr->ConstEval();
}

void Parser::EExpressionList(std::vector<pExpr>& dest) {
  do {
    dest.emplace_back(EExpression());
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
          designator.emplace_back(in_stream.Get().get<IdentifierToken>().name);
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