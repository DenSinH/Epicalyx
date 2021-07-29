#pragma once

#include "Default.h"
#include "Node.h"
#include "types/Types.h"

#include <variant>
#include <vector>
#include <string>

namespace epi {

struct InitializerList;

using Designator = std::variant<std::string, i32>;
using DesignatorList = std::vector<Designator>;
using Initializer = std::variant<pExpr, pNode<InitializerList>>;

struct InitializerList {

  void Push(DesignatorList&& member, Initializer&& value) {
    list.emplace_back(std::move(member), std::move(value));
  }

  std::vector<std::pair<DesignatorList, Initializer>> list;

  std::string to_string() const;
};


struct StructUnionType;

struct InitializerListVisitor : public TypeVisitor {
  InitializerListVisitor(const Parser& parser, InitializerList& list) : parser(parser), list(list) { }

  InitializerList& list;
  const Parser& parser;

  virtual void VisitScalar(const CType& type) = 0;
  virtual void VisitStructLike(const StructUnionType& type) = 0;

  void Visit(const VoidType& type) final { throw std::runtime_error("Invalid initializer list: cannot cast type to incomplete type void"); }
  void Visit(const ValueType<i8>& type) final { VisitScalar(type); }
  void Visit(const ValueType<u8>& type) final { VisitScalar(type); }
  void Visit(const ValueType<i16>& type) final { VisitScalar(type); }
  void Visit(const ValueType<u16>& type) final { VisitScalar(type); }
  void Visit(const ValueType<i32>& type) final { VisitScalar(type); }
  void Visit(const ValueType<u32>& type) final { VisitScalar(type); }
  void Visit(const ValueType<i64>& type) final { VisitScalar(type); }
  void Visit(const ValueType<u64>& type) final { VisitScalar(type); }
  void Visit(const ValueType<float>& type) final { VisitScalar(type); }
  void Visit(const ValueType<double>& type) final { VisitScalar(type); }
  void Visit(const PointerType& type) final { VisitScalar(type); }
  void Visit(const ArrayType& type) override = 0;
  void Visit(const FunctionType& type) final { VisitScalar(type); }
  void Visit(const StructType& type) final { VisitStructLike(type); }
  void Visit(const UnionType& type) final { VisitStructLike(type); }
};

struct ValidInitializerListVisitor final : public InitializerListVisitor {
  ValidInitializerListVisitor(const Parser& parser, InitializerList& list) :
      InitializerListVisitor(parser, list) {

  }

  void VisitScalar(const CType& type) final;
  void VisitStructLike(const StructUnionType& type) final;
  void Visit(const ArrayType& type) final;
};

struct ReduceInitializerListVisitor final : public InitializerListVisitor {
  ReduceInitializerListVisitor(const Parser& parser, InitializerList& list) :
        InitializerListVisitor(parser, list) {

  }

  pExpr reduced = nullptr;
  pExpr Reduce(const CType& type) {
    type.Visit(*this);
    return std::move(reduced);
  }

  void VisitScalar(const CType& type) final;
  void VisitStructLike(const StructUnionType& type) final { }
  void Visit(const ArrayType& type) final { }
};



}