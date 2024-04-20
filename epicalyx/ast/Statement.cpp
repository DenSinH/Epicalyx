#include "Statement.h"
#include "Declaration.h"
namespace epi { struct Parser; }
#include "Cast.h"

#include "SStream.h"

#include <regex>

namespace epi::ast {

ForNode::ForNode(std::vector<pNode<DeclarationNode>>&& decls,
         std::vector<pExpr>&& inits,
         pExpr&& cond,
         std::vector<pExpr>&& updates,
         pNode<>&& stat) :
      decls(std::move(decls)),
      inits(std::move(inits)),
      cond(std::move(cond)),
      updates(std::move(updates)),
      stat(std::move(stat)) {

}

std::string IfNode::ToString() const {
  if (_else) {
    return cotyl::FormatStr("if (%s) %s\nelse %s", cond, stat, _else);
  }
  return cotyl::FormatStr("if (%s) %s", cond, stat);
}

std::string WhileNode::ToString() const {
  return cotyl::FormatStr("while (%s) %s", cond, stat);
}

std::string DoWhileNode::ToString() const {
  return cotyl::FormatStr("do %s while (%s);", stat, cond);
}

std::string ReturnNode::ToString() const {
  if (expr) {
    return cotyl::FormatStr("return %s;", expr);
  }
  return "return;";
}

std::string ForNode::ToString() const {
  cotyl::StringStream result{};
  result << "for (";
  result << cotyl::Join(", ", decls);
  result << cotyl::Join(", ", inits);
  result << "; ";
  result << stringify(cond);
  result << "; ";
  result << cotyl::Join(", ", updates);
  result << ") ";
  result << stringify(stat);
  return result.finalize();
}

std::string SwitchNode::ToString() const {
  return cotyl::FormatStr("switch (%s) %s", expr, stat);
}

std::string CompoundNode::ToString() const {
  cotyl::StringStream repr{};
  repr << '{';
  for (const auto& stat : stats) {
    repr << '\n';
    repr << stringify(stat);
    repr << ';';
  }
  std::string result = std::regex_replace(repr.finalize(), std::regex("\n"), "\n  ");
  return result + "\n}";
}

pNode<> IfNode::Reduce() {
  auto n_cond = cond->Reduce();
  if (n_cond) cond = std::move(cond);
  auto n_stat = stat->Reduce();
  if (n_stat) stat = std::move(stat);
  if (_else) {
    auto n_else = _else->Reduce();
    if (n_else) _else = std::move(_else);
  }

  if (cond->IsConstexpr()) {
    if (cond->type.ConstBoolVal()) {
      return std::move(stat);
    }
    else if (_else) {
      return std::move(_else);
    }
    else {
      return std::make_unique<EmptyNode>();
    }
  }
  return nullptr;
}

pNode<> WhileNode::Reduce() {
  auto n_cond = cond->Reduce();
  if (n_cond) cond = std::move(n_cond);
  auto n_stat = stat->Reduce();
  if (n_stat) stat = std::move(n_stat);

  if (cond->IsConstexpr()) {
    if (!cond->type.ConstBoolVal()) {
      return std::make_unique<EmptyNode>();
    }
  }
  return nullptr;
}

pNode<> DoWhileNode::Reduce() {
  auto n_cond = cond->Reduce();
  if (n_cond) cond = std::move(n_cond);
  auto n_stat = stat->Reduce();
  if (n_stat) stat = std::move(n_stat);

  if (cond->IsConstexpr()) {
    if (!cond->type.ConstBoolVal()) {
      return stat;
    }
  }
  return nullptr;
}

pNode<> ForNode::Reduce() {
  for (auto& decl : decls) {
    decl->Reduce();
  }
  for (auto& init : inits) {
    auto n_init = init->Reduce();
    if (n_init) init = std::move(n_init);
  }
  auto n_cond = cond->Reduce();
  if (n_cond) cond = std::move(n_cond);

  for (auto& update : updates) {
    auto n_update = update->Reduce();
    if (n_update) update = std::move(n_update);
  }

  auto n_stat = stat->Reduce();
  if (n_stat) stat = std::move(n_stat);

  if (cond->IsConstexpr()) {
    if (!cond->GetType()->GetBoolValue()) {
      return std::make_unique<EmptyNode>();
    }
  }
  return nullptr;
}

pNode<> SwitchNode::Reduce() {
  auto n_expr = expr->Reduce();
  if (n_expr) expr = std::move(n_expr);
  auto n_stat = stat->Reduce();
  if (n_stat) stat = std::move(n_stat);
  return nullptr;
}

pNode<> LabelNode::Reduce() {
  auto n_stat = stat->Reduce();
  if (n_stat) stat = std::move(n_stat);
  return nullptr;
}

pNode<> CaseNode::Reduce() {
  auto n_stat = stat->Reduce();
  if (n_stat) stat = std::move(n_stat);
  return nullptr;
}

pNode<> DefaultNode::Reduce() {
  auto n_stat = stat->Reduce();
  if (n_stat) stat = std::move(n_stat);
  return nullptr;
}

pNode<> ReturnNode::Reduce() {
  auto n_expr = expr->Reduce();
  if (n_expr) expr = std::move(n_expr);
  return nullptr;
}

pNode<> CompoundNode::Reduce() {
  // reduced when added
  for (auto& node : stats) {
    if (node->IsStatement()) {
      auto n_stat = cotyl::unique_ptr_cast<StatNode>(node)->Reduce();
      if (n_stat) node = std::move(n_stat);
    }
    // don't do anything for DeclarationNodes
  }
  return nullptr;
}

}