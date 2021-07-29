#include "Parser.h"

#include "nodes/Statement.h"

namespace epi {

pNode<Stat> Parser::SStatement() {
  switch (in_stream.ForcePeek()->type) {
    case TokenType::SemiColon: {
      in_stream.Skip();
      return nullptr;
    }
    case TokenType::Case: {
      in_stream.Skip();
      auto expr = EConstexpr();
      in_stream.Eat(TokenType::Colon);

      // validation
      case_scope.Add(expr);

      return std::make_unique<Case>(expr, SStatement());
    }
    case TokenType::Default: {
      in_stream.EatSequence(TokenType::Default, TokenType::Colon);
      return std::make_unique<Default>(SStatement());
    }
    case TokenType::Switch: {
      in_stream.EatSequence(TokenType::Switch, TokenType::LParen);
      auto expr = EExpression();
      in_stream.Eat(TokenType::RParen);

      // validation
      auto expr_t = expr->GetType(*this);
      if (!expr_t->IsIntegral()) {
        throw std::runtime_error("Expression is not an integral expression");
      }

      auto stat = case_scope << [&]{ return SStatement(); };

      return std::make_unique<Switch>(std::move(expr), SStatement());
    }
    case TokenType::If: {
      in_stream.EatSequence(TokenType::If, TokenType::LParen);
      // todo: commas?
      auto cond = EExpression();
      in_stream.Eat(TokenType::RParen);

      // validation
      auto cond_t = cond->GetType(*this);
      if (!cond_t->HasTruthiness()) {
        throw std::runtime_error("Condition has no truthiness");
      }

      auto stat = SStatement();
      if (!in_stream.IsAfter(0, TokenType::Else)) {
        return std::make_unique<If>(std::move(cond), std::move(stat));
      }
      in_stream.Skip();
      return std::make_unique<If>(std::move(cond), std::move(stat), SStatement());
    }
    case TokenType::While: {
      in_stream.EatSequence(TokenType::While, TokenType::LParen);
      auto cond = EExpression();

      // validation
      auto cond_t = cond->GetType(*this);
      if (!cond_t->HasTruthiness()) {
        throw std::runtime_error("Condition has no truthiness");
      }

      in_stream.Eat(TokenType::RParen);
      loop_scope.push_back(Loop::While);
      auto stat = SStatement();
      loop_scope.pop_back();
      return std::make_unique<While>(std::move(cond), std::move(stat));
    }
    case TokenType::Do: {
      in_stream.Skip();

      loop_scope.push_back(Loop::While);
      auto stat = SStatement();
      loop_scope.pop_back();

      in_stream.EatSequence(TokenType::While, TokenType::LParen);
      auto cond = EExpression();

      // validation
      auto cond_t = cond->GetType(*this);
      if (!cond_t->HasTruthiness()) {
        throw std::runtime_error("Condition has no truthiness");
      }

      in_stream.EatSequence(TokenType::RParen, TokenType::SemiColon);
      return std::make_unique<DoWhile>(std::move(stat), std::move(cond));
    }
    case TokenType::For: {
      in_stream.EatSequence(TokenType::For, TokenType::LParen);

      // new scope for for loop declarations
      PushScope();
      std::vector<pNode<InitDeclaration>> declarations;
      if (IsDeclarationSpecifier()) {
        DInitDeclaratorList(declarations);
      }

      std::vector<pExpr> init{};
      if (!in_stream.IsAfter(0, TokenType::SemiColon)) {
        EExpressionList(init);
      }
      in_stream.Eat(TokenType::SemiColon);
      pExpr cond{};
      if (!in_stream.IsAfter(0, TokenType::SemiColon)) {
        cond = EExpression();
      }
      std::vector<pExpr> update{};
      in_stream.Eat(TokenType::SemiColon);
      if (!in_stream.IsAfter(0, TokenType::RParen)) {
        EExpressionList(update);
      }
      in_stream.Eat(TokenType::RParen);

      loop_scope.push_back(Loop::For);
      auto stat = SStatement();
      loop_scope.pop_back();

      PopScope();
      return std::make_unique<For>(
          std::move(declarations),
          std::move(init),
          std::move(cond),
          std::move(update),
          std::move(stat)
      );
    }
    case TokenType::Goto: {
      in_stream.Skip();
      in_stream.Expect(TokenType::Identifier);
      std::string label = static_cast<const tIdentifier*>(in_stream.Get().get())->name;
      in_stream.Eat(TokenType::SemiColon);

      // validation
      if (!labels.contains(label)) {
        throw cotyl::FormatExceptStr("Undefined label: %s", label);
      }

      return std::make_unique<Goto>(std::move(label));
    }
    case TokenType::Continue: {
      in_stream.Skip();
      if (loop_scope.empty()) {
        throw std::runtime_error("Cannot continue from here");
      }

      return std::make_unique<Continue>();
    }
    case TokenType::Break: {
      in_stream.Skip();
      if (loop_scope.empty() && (case_scope.Depth() == 1)) {
        throw std::runtime_error("Cannot break from here");
      }
      return std::make_unique<Break>();
    }
    case TokenType::Return: {
      // todo: check function return type
      in_stream.Skip();
      if (in_stream.IsAfter(0, TokenType::SemiColon)) {
        in_stream.Skip();
        return std::make_unique<Return>();
      }
      auto expr = EExpression();
      in_stream.Eat(TokenType::SemiColon);
      return std::make_unique<Return>(std::move(expr));
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
        std::string name = static_cast<const tIdentifier*>(in_stream.Get().get())->name;
        in_stream.Skip();
        if (labels.contains(name)) {
          throw cotyl::FormatExceptStr("Duplicate label: %s", name);
        }
        labels.insert(name);
        return std::make_unique<Label>(std::move(name), SStatement());
      }
    } [[fallthrough]];
    default: {
      // expression or declaration
      if (IsDeclarationSpecifier()) {
        throw std::runtime_error("Unexpected declaration");
      }
      std::vector<pExpr> exprlist;
      EExpressionList(exprlist);
      if (exprlist.size() == 1) {
        return std::move(exprlist[0]);
      }
      auto compound = std::make_unique<Compound>();
      for (auto& expr : exprlist) {
        expr->GetType(*this);  // validates expression
        compound->AddNode(std::move(expr));
      }
      return std::move(compound);
    }
  }
}

pNode<Compound> Parser::SCompound() {
  auto compound =  std::make_unique<Compound>();
  PushScope();
  while (!in_stream.IsAfter(0, TokenType::RBrace)) {
    if (in_stream.IsAfter(0, TokenType::StaticAssert)) {
      DStaticAssert();
    }
    else if (IsDeclarationSpecifier()) {
      std::vector<pNode<InitDeclaration>> decl_list;
      DInitDeclaratorList(decl_list);
      in_stream.Eat(TokenType::SemiColon);
      for (auto& decl : decl_list) {
        decl->VerifyAndRecord(*this);
        decl->DReduce(*this);
        compound->AddNode(std::move(decl));
      }
    }
    else {
      auto stat = SStatement();
      if (stat) {
        auto n_stat = stat->SReduce(*this);
        if (n_stat) {
          compound->AddNode(std::move(n_stat));
        }
        else {
          compound->AddNode(std::move(stat));
        }
      }
    }
  }
  PopScope();
  return compound;
}

}