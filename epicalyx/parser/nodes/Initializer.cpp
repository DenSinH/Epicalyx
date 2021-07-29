#include "Initializer.h"
#include "Expression.h"
#include "types/Types.h"
#include "parser/Parser.h"
#include "Log.h"

#include <sstream>
#include <regex>

namespace epi {


std::string InitializerList::to_string() const {
  std::stringstream repr{};
  repr << '{';
  for (const auto& init : list) {
    repr << '\n';
    for (const auto& des : init.first) {
      if (std::holds_alternative<std::string>(des)) {
        repr << '.' << std::get<std::string>(des);
      }
      else {
        repr << '[' << std::get<i32>(des) << ']';
      }
    }
    if (!init.first.empty()) {
      repr << " = ";
    }
    if (std::holds_alternative<pExpr>(init.second)) {
      repr << std::get<pExpr>(init.second)->to_string() << ',';
    }
    else {
      repr << std::get<pNode<InitializerList>>(init.second)->to_string() << ',';
    }
  }
  std::string result = std::regex_replace(repr.str(), std::regex("\n"), "\n  ");
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
  // try to cast
  type.Cast(*std::get<pExpr>(list.list[0].second)->GetType(parser));
}

void ValidInitializerListVisitor::VisitStructLike(const StructUnionType& type) {
  throw std::runtime_error("Unimplemented");
}

void ValidInitializerListVisitor::Visit(const ArrayType& type) {
  throw std::runtime_error("Unimplemented");
}

void ReduceInitializerListVisitor::VisitScalar(const CType& type) {
  // list has already been verified
  if (list.list.empty()) {
    reduced = std::make_unique<Cast>(type.Clone(), std::make_unique<NumericalConstant<i32>>(0));
  }
  else {
    reduced = std::make_unique<Cast>(type.Clone(), std::move(std::get<pExpr>(list.list[0].second)));
  }
}


}