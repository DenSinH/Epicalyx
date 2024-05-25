#pragma once

#include "Default.h"
#include "NodeFwd.h"
#include "types/TypeFwd.h"
#include "Vector.h"

#include "swl/variant.hpp"
#include <string>

namespace epi::ast {

struct InitializerList;
struct Initializer;

using Designator = swl::variant<cotyl::CString, i64>;
using DesignatorList = cotyl::vector<Designator>;

struct InitializerList {

  void Push(DesignatorList&& member, Initializer&& value);

  // might update array size
  void ValidateAndReduce(type::AnyType& type);

  cotyl::vector<std::pair<DesignatorList, Initializer>> list{};

  std::string ToString() const;
private:
  void ValidateAndReduceScalarType(type::AnyType& type);
};

struct Initializer {

  Initializer();
  Initializer(pExpr&& expr);
  Initializer(InitializerList&& init);

  void ValidateAndReduce(type::AnyType& type);

  std::string ToString() const;
  
  swl::variant<pExpr, InitializerList> value;
};

}