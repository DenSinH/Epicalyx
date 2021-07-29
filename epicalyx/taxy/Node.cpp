#include "Node.h"
#include "Statement.h"
#include "Expression.h"
#include "parser/Parser.h"


namespace epi::taxy {

pExpr Expr::EReduce(const Parser& parser) {
  return ConstTypeVisitor(parser).GetConstNode(*GetType(parser));
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