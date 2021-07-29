#include "Statement.h"
#include "Declaration.h"
#include "parser/Parser.h"

#include <regex>
#include <sstream>

namespace epi {

std::string If::to_string() const {
  if (_else) {
    return cotyl::FormatStr("if (%s) %s\nelse %s", cond, stat, _else);
  }
  return cotyl::FormatStr("if (%s) %s", cond, stat);
}

std::string While::to_string() const {
  return cotyl::FormatStr("while (%s) %s", cond, stat);
}

std::string DoWhile::to_string() const {
  return cotyl::FormatStr("do %s while (%s);", stat, cond);
}

std::string Return::to_string() const {
  if (expr) {
    return cotyl::FormatStr("return %s;", expr);
  }
  return "return;";
}

std::string For::to_string() const {
  std::stringstream result{};
  result << "for (";
  result << cotyl::Join(", ", decls);
  result << cotyl::Join(", ", inits);
  result << "; ";
  result << cond->to_string();
  result << "; ";
  result << cotyl::Join(", ", updates);
  result << ") ";
  result << stat->to_string();
  return result.str();
}

std::string Switch::to_string() const {
  return cotyl::FormatStr("switch (%s) %s", expr, stat);
}

std::string Compound::to_string() const {
  std::stringstream repr{};
  repr << '{';
  for (const auto& stat : stats) {
    repr << '\n';
    repr << stat->to_string();
    if (stat->IsDeclaration()) {
      repr << ';';
    }
  }
  std::string result = std::regex_replace(repr.str(), std::regex("\n"), "\n  ");
  return result + "\n}";
}

pNode<Stat> If::SReduce(Parser& parser) {
  auto n_cond = cond->EReduce(parser);
  if (n_cond) cond = std::move(cond);
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(stat);
  if (_else) {
    auto n_else = stat->SReduce(parser);
    if (n_else) _else = std::move(_else);
  }

  if (cond) {
    if (cond->IsConstexpr(parser)) {
      if (cond->GetType(parser)->GetBoolValue()) {
        return std::move(stat);
      }
      else if (_else) {
        return std::move(_else);
      }
      else {
        return std::make_unique<Empty>();
      }
    }
  }
  return nullptr;
}

pNode<Stat> While::SReduce(Parser& parser) {
  auto n_cond = cond->EReduce(parser);
  if (n_cond) cond = std::move(cond);
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(stat);

  if (n_cond->IsConstexpr(parser)) {
    if (!n_cond->GetType(parser)->GetBoolValue()) {
      return std::make_unique<Empty>();
    }
  }
  return nullptr;
}

pNode<Stat> DoWhile::SReduce(Parser& parser) {
  auto n_cond = cond->EReduce(parser);
  if (n_cond) cond = std::move(cond);
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(stat);

  if (n_cond->IsConstexpr(parser)) {
    if (!n_cond->GetType(parser)->GetBoolValue()) {
      return std::make_unique<Empty>();
    }
  }
  return nullptr;
}

pNode<Stat> For::SReduce(Parser& parser) {
  for (auto& decl : decls) {
    decl->DReduce(parser);
  }
  for (auto& init : inits) {
    auto n_init = init->EReduce(parser);
    if (n_init) init = std::move(n_init);
  }
  auto n_cond = cond->EReduce(parser);
  if (n_cond) cond = std::move(cond);

  for (auto& update : updates) {
    auto n_update = update->EReduce(parser);
    if (n_update) update = std::move(n_update);
  }

  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(stat);

  if (n_cond->IsConstexpr(parser)) {
    if (!n_cond->GetType(parser)->GetBoolValue()) {
      return std::make_unique<Empty>();
    }
  }
  return nullptr;
}

pNode<Stat> Switch::SReduce(Parser& parser) {
  auto n_expr = expr->EReduce(parser);
  if (n_expr) expr = std::move(n_expr);
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(stat);
  return nullptr;
}

pNode<Stat> Label::SReduce(Parser& parser) {
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(stat);
  return nullptr;
}

pNode<Stat> Case::SReduce(Parser& parser) {
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(stat);
  return nullptr;
}

pNode<Stat> Default::SReduce(Parser& parser) {
  auto n_stat = stat->SReduce(parser);
  if (n_stat) stat = std::move(stat);
  return nullptr;
}

pNode<Stat> Return::SReduce(Parser& parser) {
  auto n_expr = expr->EReduce(parser);
  if (n_expr) expr = std::move(n_expr);
  return nullptr;
}

pNode<Stat> Compound::SReduce(Parser& parser) {
  // reduced when added
//  for (auto& node : stats) {
//    if (node->IsStatement()) {
//      auto n_stat = static_cast<Stat*>(node.get())->SReduce(parser);
//      if (n_stat) node = std::move(n_stat);
//    }
//    else {
//      // declaration
//
//    }
//  }
  return nullptr;
}

}