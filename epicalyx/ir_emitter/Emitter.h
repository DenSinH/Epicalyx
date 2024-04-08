#pragma once

#include "calyx/Calyx.h"
#include "ast/Node.h"
#include "Containers.h"

#include <vector>

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
  calyx::var_index_t EmitExpr(Var var, Args... args) {
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
      case calyx::Directive::Class::Branch:
      case calyx::Directive::Class::Return:
      case calyx::Directive::Class::Select:
        reachable = false;
        break;
      default:
        break;
    }
    current_function->blocks[current_block].push_back(std::move(directive));
    return ref;
  }

  calyx::block_label_t MakeBlock() {
    // block 0 is special
    calyx::block_label_t id = current_function->blocks.size() + 1;
    current_function->blocks.emplace(id, calyx::block_t{});
    return id;
  }

  void SelectBlock(calyx::block_label_t id) {
    reachable = true;
    current_block = id;
  }

  void MakeProgram(std::vector<ast::pNode<ast::DeclNode>>& ast);

  // 0 are special IDs
  calyx::var_index_t ir_counter = 1;  // ir vars
  calyx::var_index_t c_counter = 1;   // c vars

  calyx::Function* current_function = nullptr;
  calyx::block_label_t current_block = 0;

  std::vector<Var> vars;
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

  void NewFunction(const std::string& symbol) {
    SetFunction(program.functions.emplace(symbol, symbol).first->second);
  }

private:
  bool reachable = true;
};

}