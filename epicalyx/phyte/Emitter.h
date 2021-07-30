#pragma once

#include "calyx/Calyx.h"
#include "Scope.h"
#include "taxy/Node.h"

#include <vector>
#include <stack>

namespace epi::phyte {

struct Emitter {

  enum class State {
    Assign,
    Address,
    Read,
  };

  // todo: microblocks
  template<typename T, typename... Args>
  calyx::var_index_t EmitExpr(Args... args) {
    auto expr_idx = ir_counter++;
    program.push_back(std::make_unique<T>(expr_idx, args...));
    return expr_idx;
  }

  template<typename T, typename... Args>
  void Emit(Args... args) {
    program.push_back(std::make_unique<T>(args...));
  }

  void MakeProgram(std::vector<taxy::pNode<taxy::Decl>>& ast);

  // 0 are special IDs
  calyx::var_index_t ir_counter = 1;
  calyx::var_index_t c_counter = 1;

  std::stack<State> state{};
  cotyl::MapScope<std::string, calyx::CVar> variables{};
  std::vector<calyx::pIROp> program{};
};

}