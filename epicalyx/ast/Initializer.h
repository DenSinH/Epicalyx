#pragma once

#include "Default.h"
#include "Node.h"
#include "types/BaseType.h"

#include <variant>
#include <vector>
#include <string>

namespace epi {
struct StructUnionType;
}

namespace epi::ast {

struct InitializerList;
struct Initializer;

using Designator = std::variant<cotyl::CString, i64>;
using DesignatorList = std::vector<Designator>;

struct InitializerList {

  void Push(DesignatorList&& member, Initializer&& value);

  std::vector<std::pair<DesignatorList, Initializer>> list;

  std::string ToString() const;
};

struct Initializer {

  void Reduce();
  std::string ToString() const;
  
private:
  std::variant<pExpr, InitializerList> value;
};


// struct InitializerListVisitor : public TypeVisitor {
//   InitializerListVisitor(InitializerList& list) : list{list} { }

//   InitializerList& list;

//   virtual void VisitScalar(const CType& type) = 0;
//   virtual void VisitStructLike(const StructUnionType& type) = 0;

//   void Visit(const VoidType& type) final;
//   void Visit(const ValueType<i8>& type) final;
//   void Visit(const ValueType<u8>& type) final;
//   void Visit(const ValueType<i16>& type) final;
//   void Visit(const ValueType<u16>& type) final;
//   void Visit(const ValueType<i32>& type) final;
//   void Visit(const ValueType<u32>& type) final;
//   void Visit(const ValueType<i64>& type) final;
//   void Visit(const ValueType<u64>& type) final;
//   void Visit(const ValueType<float>& type) final;
//   void Visit(const ValueType<double>& type) final;
//   void Visit(const PointerType& type) final;
//   void Visit(const ArrayType& type) override = 0;
//   void Visit(const FunctionType& type) final;
//   void Visit(const StructType& type) final;
//   void Visit(const UnionType& type) final;
// };

// struct ValidInitializerListVisitor final : public InitializerListVisitor {
  
//   ValidInitializerListVisitor(const ConstParser& parser, InitializerList& list) :
//         InitializerListVisitor(list), parser{parser} {

//   }

//   const ConstParser& parser;

//   void VisitScalar(const CType& type) final;
//   void VisitStructLike(const StructUnionType& type) final;
//   void Visit(const ArrayType& type) final;
// };

// struct ReduceInitializerListVisitor final : public InitializerListVisitor {
//   ReduceInitializerListVisitor(const Parser& parser, InitializerList& list) :
//         InitializerListVisitor(list), parser{parser} {

//   }

//   const Parser& parser;

//   pExpr reduced = nullptr;
//   pExpr Reduce(const CType& type) {
//     type.Visit(*this);
//     return std::move(reduced);
//   }

//   void VisitScalar(const CType& type) final;
//   void VisitStructLike(const StructUnionType& type) final { }
//   void Visit(const ArrayType& type) final { }
// };



}