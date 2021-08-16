#pragma once

#include "calyx/Calyx.h"
#include "taxy/Node.h"

#include <vector>

namespace epi::phyte {

struct Emitter {

  Emitter();

  // todo: blocks
  template<typename T, typename... Args>
  calyx::var_index_t EmitExpr(calyx::Var var, Args... args) {
    vars.emplace_back(var);
    auto expr_idx = ir_counter++;
    Emit<T>(expr_idx, args...);
    return expr_idx;
  }

  template<typename T, typename... Args>
  void Emit(Args... args) {
    program[current_block].push_back(std::make_unique<T>(args...));
  }

  calyx::block_label_t MakeBlock() {
    calyx::block_label_t id = program.size();
    program.emplace_back();
    return id;
  }

  void SelectBlock(calyx::block_label_t id) {
    current_block = id;
  }

  void MakeProgram(std::vector<taxy::pNode<taxy::Decl>>& ast);

  // 0 are special IDs
  calyx::var_index_t ir_counter = 1;  // ir vars
  calyx::var_index_t c_counter = 1;   // c vars

  calyx::block_label_t current_block = 0;

  // first var is special
  std::vector<calyx::Var> vars{{calyx::Var::Type::I32}};
  calyx::Program program{};
};

}