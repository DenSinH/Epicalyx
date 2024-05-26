#pragma once

#include <string>                      // for string
#include <utility>                     // for pair

#include "CString.h"                   // for CString
#include "Default.h"                   // for i64
#include "NodeFwd.h"                   // for pExpr
#include "Vector.h"                    // for vector
#include "swl/variant.hpp"             // for variant

namespace epi { namespace type { struct AnyType; } }

namespace epi::ast {

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