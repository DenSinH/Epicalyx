#include "Parser.h"

#include "Stream.h"
#include "tokenizer/Token.h"
#include "ast/Statement.h"
#include "ast/Declaration.h"

namespace epi {
  
using namespace ast;

static pStat StatOrReduced(pStat&& stat) {
  auto reduced = stat->Reduce();
  if (reduced) return std::move(reduced);
  return std::move(stat);
}

pNode<StatNode> Parser::SStatement() {
  switch (in_stream.ForcePeek()->type) {
    case TokenType::SemiColon: {
      in_stream.Skip();
      return std::make_unique<EmptyNode>();
    }
    case TokenType::Case: {
      in_stream.Skip();
      auto expr = EConstexpr();
      in_stream.Eat(TokenType::Colon);

      // validation
      if (case_scope.HasTop(expr)) {
        throw std::runtime_error("Duplicate case value");
      }
      case_scope.Add(expr);

      auto case_stat = std::make_unique<CaseNode>(expr, SStatement());
      return case_stat;
    }
    case TokenType::Default: {
      in_stream.EatSequence(TokenType::Default, TokenType::Colon);
      auto def_stat = std::make_unique<DefaultNode>(SStatement());

      return def_stat;
    }
    case TokenType::Switch: {
      in_stream.EatSequence(TokenType::Switch, TokenType::LParen);
      auto expr = EExpression();
      in_stream.Eat(TokenType::RParen);

      auto stat = case_scope << [&]{ return SStatement(); };
      auto switch_stat = std::make_unique<SwitchNode>(std::move(expr), std::move(stat));

      return switch_stat;
    }
    case TokenType::If: {
      in_stream.EatSequence(TokenType::If, TokenType::LParen);
      // todo: commas?
      auto cond = EExpression();
      in_stream.Eat(TokenType::RParen);

      auto stat = SStatement();
      if (!in_stream.IsAfter(0, TokenType::Else)) {
        return std::make_unique<IfNode>(std::move(cond), std::move(stat));
      }
      in_stream.Skip();

      auto if_stat = std::make_unique<IfNode>(std::move(cond), std::move(stat), SStatement());

      return StatOrReduced(std::move(if_stat));
    }
    case TokenType::While: {
      in_stream.EatSequence(TokenType::While, TokenType::LParen);
      auto cond = EExpression();

      in_stream.Eat(TokenType::RParen);
      loop_scope.push_back(Loop::While);
      auto stat = SStatement();
      loop_scope.pop_back();
      auto while_stat = std::make_unique<WhileNode>(std::move(cond), std::move(stat));

      return StatOrReduced(std::move(while_stat));
    }
    case TokenType::Do: {
      in_stream.Skip();

      loop_scope.push_back(Loop::While);
      auto stat = SStatement();
      loop_scope.pop_back();

      in_stream.EatSequence(TokenType::While, TokenType::LParen);
      auto cond = EExpression();

      in_stream.EatSequence(TokenType::RParen, TokenType::SemiColon);
      auto dowhile_stat = std::make_unique<DoWhileNode>(std::move(stat), std::move(cond));

      return StatOrReduced(std::move(dowhile_stat));
    }
    case TokenType::For: {
      in_stream.EatSequence(TokenType::For, TokenType::LParen);

      // new scope for for loop declarations
      PushScope();
      cotyl::vector<pNode<DeclarationNode>> decl_list;
      if (IsDeclarationSpecifier()) {
        DInitDeclaratorList(decl_list);
      }

      cotyl::vector<pExpr> init{};
      if (!in_stream.IsAfter(0, TokenType::SemiColon)) {
        EExpressionList(init);
      }
      in_stream.Eat(TokenType::SemiColon);
      pExpr cond{};
      if (!in_stream.IsAfter(0, TokenType::SemiColon)) {
        cond = EExpression();
      }
      cotyl::vector<pExpr> update{};
      in_stream.Eat(TokenType::SemiColon);
      if (!in_stream.IsAfter(0, TokenType::RParen)) {
        EExpressionList(update);
      }
      in_stream.Eat(TokenType::RParen);

      loop_scope.push_back(Loop::For);
      auto stat = SStatement();
      loop_scope.pop_back();

      auto for_stat = std::make_unique<ForNode>(
          std::move(decl_list),
          std::move(init),
          std::move(cond),
          std::move(update),
          std::move(stat)
      );

      PopScope();
      return StatOrReduced(std::move(for_stat));
    }
    case TokenType::Goto: {
      in_stream.Skip();
      in_stream.Expect(TokenType::Identifier);
      auto label = std::move(in_stream.Get().get<IdentifierToken>().name);
      in_stream.Eat(TokenType::SemiColon);

      // labels might be predeclared
      if (!labels.contains(label)) {
        unresolved_labels.insert(label);
      }

      return std::make_unique<GotoNode>(std::move(label));
    }
    case TokenType::Continue: {
      in_stream.Skip();
      if (loop_scope.empty()) {
        throw std::runtime_error("Cannot continue from here");
      }

      return std::make_unique<ContinueNode>();
    }
    case TokenType::Break: {
      in_stream.Skip();
      if (loop_scope.empty() && (case_scope.Depth() == 1)) {
        throw std::runtime_error("Cannot break from here");
      }
      return std::make_unique<BreakNode>();
    }
    case TokenType::Return: {
      in_stream.Skip();
      if (in_stream.IsAfter(0, TokenType::SemiColon)) {
        in_stream.Skip();
        return std::make_unique<ReturnNode>();
      }
      auto expr = EExpression();
      in_stream.Eat(TokenType::SemiColon);

      // check function return type
      function_return->Cast(expr->type, false);

      auto ret_stat = std::make_unique<ReturnNode>(std::move(expr));
      return ret_stat;
    }
    case TokenType::LBrace: {
      // compound
      in_stream.Skip();
      auto compound = SCompound();
      in_stream.Eat(TokenType::RBrace);
      return compound;
    }
    case TokenType::Identifier: {
      if (in_stream.IsAfter(1, TokenType::Colon)) {
        // label
        auto name = std::move(in_stream.Get().get<IdentifierToken>().name);
        in_stream.Skip();
        if (labels.contains(name)) {
          throw cotyl::FormatExcept("Duplicate label: %s", name.c_str());
        }
        labels.insert(name);
        return std::make_unique<LabelNode>(std::move(name), SStatement());
      }
    } [[fallthrough]];
    default: {
      // expression or declaration
      if (IsDeclarationSpecifier()) {
        throw std::runtime_error("Unexpected declaration");
      }
      cotyl::vector<pExpr> exprlist{};
      EExpressionList(exprlist);
      if (exprlist.size() == 1) {
        return std::move(exprlist[0]);
      }
      auto compound = std::make_unique<CompoundNode>();
      for (auto& expr : exprlist) {
        compound->AddNode(std::move(expr));
      }
      return std::move(compound);
    }
  }
}

pNode<CompoundNode> Parser::SCompound() {
  auto compound =  std::make_unique<CompoundNode>();
  PushScope();
  while (!in_stream.IsAfter(0, TokenType::RBrace)) {
    if (in_stream.IsAfter(0, TokenType::StaticAssert)) {
      DStaticAssert();
    }
    else if (IsDeclarationSpecifier()) {
      cotyl::vector<pNode<DeclarationNode>> decl_list{};
      DInitDeclaratorList(decl_list);
      in_stream.Eat(TokenType::SemiColon);
      for (auto& decl : decl_list) {
        RecordDeclaration(*decl);
        compound->AddNode(std::move(decl));
      }
    }
    else {
      auto stat = SStatement();
      if (stat) {
        compound->AddNode(std::move(stat));
      }
    }
  }
  PopScope();
  return compound;
}

}