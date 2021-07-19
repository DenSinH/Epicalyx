#pragma once

#include "nodes/Node.h"

namespace epi {

struct Token;
struct tIdentifier;
template<typename T> struct tNumericConstant;
struct tStringConstant;


struct ConstTokenVisitor {

  pExpr Visit(const Token& tok);
  pExpr Visit(const tIdentifier& tok);
  template<typename T>
  pExpr Visit(const tNumericConstant<T>& tok);
  pExpr Visit(const tStringConstant& tok);
};

}