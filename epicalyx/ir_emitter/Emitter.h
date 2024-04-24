#pragma once

#include "calyx/CalyxFwd.h"
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
  requires (std::is_base_of_v<calyx::Expr, T>)
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
  requires (std::is_base_of_v<calyx::Directive, T>)
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

  void MakeProgram(cotyl::vector<ast::DeclarationNode>& decls, cotyl::vector<ast::FunctionDefinitionNode>& funcs);

  // 0 are special IDs
  var_index_t ir_counter = 1;  // ir vars
  var_index_t c_counter = 1;   // c vars

  calyx::Function* current_function = nullptr;
  block_label_t current_block = 0;

  cotyl::vector<Var> vars{};
  calyx::Program program{};

  // make new block and return block ID
  block_label_t MakeBlock();

  // select block by ID to start emitting in
  // asserts that this block is still empty
  void SelectBlock(block_label_t id);

  // set function to emit
  // asserts that this function is still empty
  void SetFunction(calyx::Function& func);

  // create new function
  void NewFunction(cotyl::CString&& symbol);

private:
  bool reachable = true;
};

}