#pragma once

#include "ast/Node.h"

namespace epi {

using namespace ast;

struct Token;
struct IdentifierToken;
template<typename T> struct NumericalConstantToken;
struct StringConstantToken;


struct ConstTokenVisitor {

  pExpr Visit(const Token& tok);
  pExpr Visit(const IdentifierToken& tok);
  template<typename T>
  pExpr Visit(const NumericalConstantToken<T>& tok);
  pExpr Visit(const StringConstantToken& tok);
};

}