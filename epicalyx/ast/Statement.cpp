#include "Statement.h"

#include <regex>                       // for regex_replace, regex
#include <utility>                     // for move

#include "Vector.h"                    // for vec_iterator
#include "Declaration.h"               // for DeclarationNode
#include "Format.h"                    // for FormatStr, Format, Join
#include "Node.h"                      // for ExprNode, Node, StatNode, stri...
#include "SStream.h"                   // for StringStream
#include "Stringify.h"                 // for stringify


namespace epi::ast {

IfNode::IfNode(pExpr&& cond, pStat&& stat, pStat&& _else) :
    cond(std::move(cond)),
    stat(std::move(stat)),
    _else(std::move(_else)) {
  this->cond->VerifyTruthiness();
}

std::string IfNode::ToString() const {
  if (_else) {
    return cotyl::FormatStr("if (%s) %s\nelse %s", cond, stat, _else);
  }
  return cotyl::FormatStr("if (%s) %s", cond, stat);
}

pStat IfNode::Reduce() {
  if (cond->IsConstexpr()) {
    if (cond->ConstBoolVal()) {
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


WhileNode::WhileNode(pExpr&& cond, pStat&& stat) :
    cond(std::move(cond)),
    stat(std::move(stat)) {
  this->cond->VerifyTruthiness();
}

pStat WhileNode::Reduce() {
  if (cond->IsConstexpr()) {
    if (!cond->ConstBoolVal()) {
      return std::make_unique<EmptyNode>();
    }
  }
  return nullptr;
}

std::string WhileNode::ToString() const {
  return cotyl::FormatStr("while (%s) %s", cond, stat);
}


DoWhileNode::DoWhileNode(pStat&& stat, pExpr&& cond) :
    stat(std::move(stat)),
    cond(std::move(cond)) {
  this->cond->VerifyTruthiness();
}

std::string DoWhileNode::ToString() const {
  return cotyl::FormatStr("do %s while (%s);", stat, cond);
}

pStat DoWhileNode::Reduce() {
  if (cond->IsConstexpr()) {
    if (!cond->ConstBoolVal()) {
      // execute once
      return std::move(stat);
    }
  }
  return nullptr;
}


ForNode::ForNode(
  cotyl::vector<DeclarationNode>&& decls,
  pExpr&& init,
  pExpr&& cond,
  pExpr&& update,
  pStat&& stat
) : decls{std::move(decls)},
    init{std::move(init)},
    cond{std::move(cond)},
    update{std::move(update)},
    stat{std::move(stat)} {
  if (this->cond) {
    this->cond->VerifyTruthiness();
  }
}

std::string ForNode::ToString() const {
  cotyl::StringStream result{};
  result << "for (";
  result << cotyl::Join(", ", decls);
  if (init) result << stringify(init);
  result << "; ";
  if (cond) result << stringify(cond);
  result << "; ";
  if (update) result << stringify(update);
  result << ") ";
  result << stringify(stat);
  return result.finalize();
}

pStat ForNode::Reduce() {
  // todo: improve deductions
  // make sure that init / update blocks are still called
  // if (cond && cond->IsConstexpr() && !cond->ConstBoolVal()) {
  //   return std::make_unique<EmptyNode>();
  // }
  return nullptr;
}


LabelNode::LabelNode(cotyl::CString&& name, pStat stat) :
    name(std::move(name)),
    stat(std::move(stat)) {

}

std::string LabelNode::ToString() const { 
  return cotyl::FormatStr("%s: %s", name.str(), stat); 
}


SwitchNode::SwitchNode(pExpr&& expr, pStat&& stat) :
        expr(std::move(expr)),
        stat(std::move(stat)) {
  this->expr->VerifySwitchable();
}

std::string SwitchNode::ToString() const {
  return cotyl::FormatStr("switch (%s) %s", expr, stat);
}


CaseNode::CaseNode(i64 expr, pStat&& stat) :
    expr(expr),
    stat(std::move(stat)) {

}

std::string CaseNode::ToString() const { 
  return cotyl::FormatStr("case %s: %s", expr, stat); 
}


DefaultNode::DefaultNode(pStat&& stat) :
    stat(std::move(stat)) {

}

std::string DefaultNode::ToString() const {
  return cotyl::FormatStr("default: %s", stat); 
}

GotoNode::GotoNode(cotyl::CString&& label) : 
    label(std::move(label)) {

}

std::string GotoNode::ToString() const { 
  return cotyl::Format("goto %s;", label.c_str()); 
}


ReturnNode::ReturnNode(pExpr expr) : 
    expr(std::move(expr)) {

}

std::string ReturnNode::ToString() const {
  if (expr) {
    return cotyl::FormatStr("return %s;", expr);
  }
  return "return;";
}


std::string BreakNode::ToString() const { 
  return "break;"; 
}


std::string ContinueNode::ToString() const { 
  return "continue;"; 
}


void CompoundNode::AddNode(pNode<Node>&& stat) {
  if (stat) stats.push_back(std::move(stat));
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

}