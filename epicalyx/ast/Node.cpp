#include "Node.h"
#include "Statement.h"
#include "Expression.h"
#include "parser/Parser.h"


namespace epi::ast {

pExpr Expr::EReduce(const Parser& parser) {
  return ConstTypeVisitor(parser).GetConstNode(*SemanticAnalysis(parser));
}


//pNode<Stat> Stat::Analyze(Parser& parser) {
//  auto impl = AnalyzeImpl(parser);
//  if (parser.unreachable) {
//    return std::make_unique<Empty>();
//  }
//
//  return impl;
//}

}