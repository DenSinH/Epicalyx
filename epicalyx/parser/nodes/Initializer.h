#pragma once

#include "Default.h"
#include "Node.h"

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

}