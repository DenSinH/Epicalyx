#include "Statement.h"
#include "Declaration.h"
#include "parser/Parser.h"

#include "Cast.h"

#include "SStream.h"

#include <regex>

namespace epi::ast {

For::For(std::vector<pNode<Declaration>>&& decls,
         std::vector<pExpr>&& inits,
         pExpr&& cond,
         std::vector<pExpr>&& updates,
         pNode<Stat>&& stat) :
      decls(std::move(decls)),
      inits(std::move(inits)),
      cond(std::move(cond)),
      updates(std::move(updates)),
      stat(std::move(stat)) {

}

std::string If::ToString() const {
  if (_else) {
    return cotyl::FormatStr("if (%s) %s\nelse %s", cond, stat, _else);
  }
  return cotyl::FormatStr("if (%s) %s", cond, stat);
}

std::string While::ToString() const {
  return cotyl::FormatStr("while (%s) %s", cond, stat);
}

std::string DoWhile::ToString() const {
  return cotyl::FormatStr("do %s while (%s);", stat, cond);
}

std::string Return::ToString() const {
  if (expr) {
    return cotyl::FormatStr("return %s;", expr);
  }
  return "return;";
}

std::string For::ToString() const {
  cotyl::StringStream result{};
  result << "for (";
  result << cotyl::Join(", ", decls);
  result << cotyl::Join(", ", inits);
  result << "; ";
  result << cond->ToString();
  result << "; ";
  result << cotyl::Join(", ", updates);
  result << ") ";
  result << stat->ToString();
  return result.finalize();
}

std::string Switch::ToString() const {
  return cotyl::FormatStr("switch (%s) %s", expr, stat);
}

std::string Compound::ToString() const {
  cotyl::StringStream repr{};
  repr << '{';
  for (const auto& stat : stats) {
    repr << '\n';
    repr << stat->ToString();
    if (stat->IsDeclaration()) {
      repr << ';';
    }
  }
  std::string result = std::regex_replace(repr.finalize(), std::regex("\n"), "\n  ");
  return result + "\n}";
}

pNode<Stat> If::SReduce(const Parser& parser) {
  auto n_cond = cond->EReduce(parser);
  if (n_cond) cond = std::move(cond);
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(stat);
  if (_else) {
    auto n_else = _else->SReduce(parser);
    if (n_else) _else = std::move(_else);
  }

  if (cond->IsConstexpr()) {
    std::printf("Constexpr cond %d\n", cond->GetType()->GetBoolValue());
    if (cond->GetType()->GetBoolValue()) {
      return std::move(stat);
    }
    else if (_else) {
      return std::move(_else);
    }
    else {
      return std::make_unique<Empty>();
    }
  }
  return nullptr;
}

pNode<Stat> While::SReduce(const Parser& parser) {
  auto n_cond = cond->EReduce(parser);
  if (n_cond) cond = std::move(n_cond);
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(n_stat);

  if (cond->IsConstexpr()) {
    if (!cond->GetType()->GetBoolValue()) {
      return std::make_unique<Empty>();
    }
  }
  return nullptr;
}

pNode<Stat> DoWhile::SReduce(const Parser& parser) {
  auto n_cond = cond->EReduce(parser);
  if (n_cond) cond = std::move(n_cond);
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(n_stat);

  if (cond->IsConstexpr()) {
    if (!cond->GetType()->GetBoolValue()) {
      return std::make_unique<Empty>();
    }
  }
  return nullptr;
}

pNode<Stat> For::SReduce(const Parser& parser) {
  for (auto& decl : decls) {
    decl->DReduce(parser);
  }
  for (auto& init : inits) {
    auto n_init = init->EReduce(parser);
    if (n_init) init = std::move(n_init);
  }
  auto n_cond = cond->EReduce(parser);
  if (n_cond) cond = std::move(n_cond);

  for (auto& update : updates) {
    auto n_update = update->EReduce(parser);
    if (n_update) update = std::move(n_update);
  }

  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(n_stat);

  if (cond->IsConstexpr()) {
    if (!cond->GetType()->GetBoolValue()) {
      return std::make_unique<Empty>();
    }
  }
  return nullptr;
}

pNode<Stat> Switch::SReduce(const Parser& parser) {
  auto n_expr = expr->EReduce(parser);
  if (n_expr) expr = std::move(n_expr);
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(n_stat);
  return nullptr;
}

pNode<Stat> Label::SReduce(const Parser& parser) {
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(n_stat);
  return nullptr;
}

pNode<Stat> Case::SReduce(const Parser& parser) {
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(n_stat);
  return nullptr;
}

pNode<Stat> Default::SReduce(const Parser& parser) {
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(n_stat);
  return nullptr;
}

pNode<Stat> Return::SReduce(const Parser& parser) {
  auto n_expr = expr->EReduce(parser);
  if (n_expr) expr = std::move(n_expr);
  return nullptr;
}

pNode<Stat> Compound::SReduce(const Parser& parser) {
  // reduced when added
  for (auto& node : stats) {
    if (node->IsStatement()) {
      auto n_stat = cotyl::unique_ptr_cast<Stat>(node)->SReduce(parser);
      if (n_stat) node = std::move(n_stat);
    }
    // don't do anything for declarations
  }
  return nullptr;
}

}