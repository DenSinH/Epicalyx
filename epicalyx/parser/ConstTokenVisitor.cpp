#include "ConstTokenVisitor.h"
#include "ast/Expression.h"
#include "tokenizer/Token.h"


namespace epi {

pExpr ConstTokenVisitor::Visit(const Token& tok) {
  throw std::runtime_error("Visited node is not a constant expression");
}

pExpr ConstTokenVisitor::Visit(const IdentifierToken& tok) {
  return std::make_unique<IdentifierNode>(tok.name);
}

template<typename T>
pExpr ConstTokenVisitor::Visit(const NumericalConstantToken<T>& tok) {
  return std::make_unique<NumericalConstantNode<T>>(tok.value);
}

template pExpr ConstTokenVisitor::Visit(const NumericalConstantToken<i32>&);
template pExpr ConstTokenVisitor::Visit(const NumericalConstantToken<u32>&);
template pExpr ConstTokenVisitor::Visit(const NumericalConstantToken<i64>&);
template pExpr ConstTokenVisitor::Visit(const NumericalConstantToken<u64>&);
template pExpr ConstTokenVisitor::Visit(const NumericalConstantToken<float>&);
template pExpr ConstTokenVisitor::Visit(const NumericalConstantToken<double>&);

pExpr ConstTokenVisitor::Visit(const StringConstantToken& tok) {
  return std::make_unique<StringConstantNode>(tok.value);
}

}