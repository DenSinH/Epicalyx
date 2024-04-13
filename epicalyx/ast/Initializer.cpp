#include "Initializer.h"
#include "Expression.h"
#include "types/Types.h"
#include "parser/Parser.h"
#include "Log.h"
#include "Exceptions.h"
#include "SStream.h"

#include <regex>

namespace epi::ast {


std::string InitializerList::ToString() const {
  cotyl::StringStream repr{};
  repr << '{';
  for (const auto& init : list) {
    repr << '\n';
    for (const auto& des : init.first) {
      if (std::holds_alternative<std::string>(des)) {
        repr << '.' << std::get<std::string>(des);
      }
      else {
        repr << '[' << std::to_string(std::get<i64>(des)) << ']';
      }
    }
    if (!init.first.empty()) {
      repr << " = ";
    }
    if (std::holds_alternative<pExpr>(init.second)) {
      repr << stringify(std::get<pExpr>(init.second)) << ',';
    }
    else {
      repr << stringify(std::get<pNode<InitializerList>>(init.second)) << ',';
    }
  }
  std::string result = std::regex_replace(repr.finalize(), std::regex("\n"), "\n  ");
  return result + "\n}";
}

void ValidInitializerListVisitor::VisitScalar(const CType& type) {
  if (list.list.empty()) {
    return;
  }
  if (list.list.size() > 1) {
    Log::Warn("Excess elements in initializer list") << [&]{
      parser.PrintLoc();
    };
  }
  if (!list.list[0].first.empty()) {
    throw std::runtime_error("Bad initializer list: no declarators expected");
  }
  if (!std::holds_alternative<pExpr>(list.list[0].second)) {
    throw std::runtime_error("Expected expression, got initializer list");
  }
  // try to CastNode
  type.Cast(*std::get<pExpr>(list.list[0].second)->GetType());
}

void ValidInitializerListVisitor::VisitStructLike(const StructUnionType& type) {
  throw cotyl::UnimplementedException();
}

void ValidInitializerListVisitor::Visit(const ArrayType& type) {
  throw cotyl::UnimplementedException();
}

void ReduceInitializerListVisitor::VisitScalar(const CType& type) {
  // list has already been verified
  if (list.list.empty()) {
    reduced = std::make_unique<CastNode>(type.Clone(), std::make_unique<NumericalConstantNode<i32>>(0));
  }
  else {
    auto expr = std::move(std::get<pExpr>(list.list[0].second));
    auto n_expr = expr->EReduce(parser);
    if (n_expr) {
      reduced = std::make_unique<CastNode>(type.Clone(), std::move(n_expr));
      auto n_reduced = reduced->EReduce(parser);
      if (n_reduced) reduced = std::move(n_reduced);
    }
    else {
      reduced = std::make_unique<CastNode>(type.Clone(), std::move(expr));
      auto n_reduced = reduced->EReduce(parser);
      if (n_reduced) reduced = std::move(n_reduced);
    }
  }
}


}