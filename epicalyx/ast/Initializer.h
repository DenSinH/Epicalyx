#pragma once

#include "Default.h"
#include "NodeFwd.h"
#include "types/TypeFwd.h"
#include "Vector.h"

#include <variant>
#include <string>

namespace epi::ast {

struct InitializerList;
struct Initializer;

using Designator = std::variant<cotyl::CString, i64>;
using DesignatorList = cotyl::vector<Designator>;

struct InitializerList {

  void Push(DesignatorList&& member, Initializer&& value);

  void ValidateAndReduce(const type::AnyType& type);

  cotyl::vector<std::pair<DesignatorList, Initializer>> list{};

  std::string ToString() const;
private:
  void ValidateAndReduceScalarType(const type::AnyType& type);
};

struct Initializer {

  Initializer();
  Initializer(pExpr&& expr);
  Initializer(InitializerList&& init);

  void ValidateAndReduce(const type::AnyType& type);

  std::string ToString() const;
  
  std::variant<pExpr, InitializerList> value;
};

}