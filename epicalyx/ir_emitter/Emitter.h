#pragma once

#include "calyx/Calyx.h"
#include "ast/NodeFwd.h"
#include "Containers.h"
#include "TypeTraits.h"
#include "Vector.h"


namespace epi {

struct Emitter {

  Emitter();

  struct Var {
    enum class Type {
      I32, U32, I64, U64, Float, Double, Pointer, Struct
    };

    Var(Type type, u64 stride = 0) :
        type(type), stride(stride) {

    }

    Type type;
    union {
      u64 stride;  // for pointers
      u64 size;    // for structs
    };
  };

  template<typename T, typename... Args>
  var_index_t EmitExpr(Var var, Args&&... args) {
    if (!reachable) {
      return 0;
    }
    vars.emplace_back(var);
    auto expr_idx = ir_counter++;
    Emit<T>(expr_idx, std::forward<Args>(args)...);
    return expr_idx;
  }

  template<typename T, typename... Args>
  T* Emit(Args&&... args) {
    if (!reachable) {
      return nullptr;
    }
    calyx::AnyDirective directive = T(std::forward<Args>(args)...);
    if (calyx::IsBlockEnd(directive)) {
      reachable = false;
    }
    current_function->blocks[current_block].push_back(std::move(directive));
    return &current_function->blocks[current_block].back().get<T>();
  }

  block_label_t MakeBlock() {
    // block 0 is special
    return current_function->AddBlock().first;
  }

  void SelectBlock(block_label_t id) {
    cotyl::Assert(current_function->blocks.at(id).empty(), "Expected empty block on selection");
    reachable = true;
    current_block = id;
  }

  void MakeProgram(cotyl::vector<ast::pNode<ast::DeclNode>>& ast);

  // 0 are special IDs
  var_index_t ir_counter = 1;  // ir vars
  var_index_t c_counter = 1;   // c vars

  calyx::Function* current_function = nullptr;
  block_label_t current_block = 0;

  cotyl::vector<Var> vars{};
  calyx::Program program{};

  void SetFunction(calyx::Function& func) {
    current_function = &func;
    const auto entry = MakeBlock();
    cotyl::Assert(entry == calyx::Function::Entry, "Expected function entry to be block 1");
    SelectBlock(entry);
    ir_counter = 1;
    c_counter = 1;
    
    // first var is special
    vars = {{Var::Type::I32}};
  }

  void NewFunction(cotyl::CString&& symbol) {
    auto func = calyx::Function{std::move(symbol)};
    const auto& sym = func.symbol;
    SetFunction(program.functions.emplace(sym, std::move(func)).first->second);
  }

private:
  bool reachable = true;
};

}