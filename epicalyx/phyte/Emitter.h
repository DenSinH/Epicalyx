#pragma once

#include "calyx/Calyx.h"
#include "taxy/Node.h"

#include <vector>
#include <unordered_map>

namespace epi::phyte {

struct Emitter {

  Emitter();

  template<typename T, typename... Args>
  calyx::var_index_t EmitExpr(calyx::Var var, Args... args) {
    if (!reachable) {
      return 0;
    }
    vars.emplace_back(var);
    auto expr_idx = ir_counter++;
    Emit<T>(expr_idx, args...);
    return expr_idx;
  }

  template<typename T, typename... Args>
  T* Emit(Args... args) {
    if (!reachable) {
      return nullptr;
    }
    auto directive = std::make_unique<T>(args...);
    T* ref = directive.get();
    switch (ref->cls) {
      case calyx::Directive::Class::UnconditionalBranch:
      case calyx::Directive::Class::Return:
      case calyx::Directive::Class::Select:
        reachable = false;
        break;
      default:
        break;
    }
    program.blocks[current_block].push_back(std::move(directive));
    return ref;
  }

  calyx::block_label_t MakeBlock() {
    calyx::block_label_t id = program.blocks.size();
    program.blocks.emplace(id, calyx::Program::block_t{});
    return id;
  }

  void SelectBlock(calyx::block_label_t id) {
    reachable = true;
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

private:
  bool reachable = true;
};

}