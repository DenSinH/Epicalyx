#include "ConstTokenVisitor.h"
#include "nodes/Expression.h"
#include "tokenizer/Token.h"


namespace epi {

pExpr ConstTokenVisitor::Visit(const Token& tok) {
  throw std::runtime_error("Visited node is not a constant expression");
}

pExpr ConstTokenVisitor::Visit(const tIdentifier& tok) {
  return std::make_unique<Identifier>(tok.name);
}

template<typename T>
pExpr ConstTokenVisitor::Visit(const tNumericConstant<T>& tok) {
  return std::make_unique<NumericalConstant<T>>(tok.value);
}

template pExpr ConstTokenVisitor::Visit(const tNumericConstant<i32>&);
template pExpr ConstTokenVisitor::Visit(const tNumericConstant<u32>&);
template pExpr ConstTokenVisitor::Visit(const tNumericConstant<i64>&);
template pExpr ConstTokenVisitor::Visit(const tNumericConstant<u64>&);
template pExpr ConstTokenVisitor::Visit(const tNumericConstant<float>&);
template pExpr ConstTokenVisitor::Visit(const tNumericConstant<double>&);

pExpr ConstTokenVisitor::Visit(const tStringConstant& tok) {
  return std::make_unique<StringConstant>(tok.value);
}

}