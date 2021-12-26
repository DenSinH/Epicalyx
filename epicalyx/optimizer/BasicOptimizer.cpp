#include "BasicOptimizer.h"
#include "IRCompare.h"
#include "Cast.h"
#include "Is.h"
#include "Containers.h"


namespace epi {

void BasicOptimizer::TryReplace(calyx::var_index_t& var_idx) const {
  if (replacement.contains(var_idx)) {
    var_idx = replacement.at(var_idx);
  }
}

template<typename T, class F>
bool BasicOptimizer::FindExprResultReplacement(T& op, F predicate) {
  for (const auto& [var_idx, loc] : vars_found) {
    auto& directive = new_program.blocks.at(loc.first)[loc.second];
    if (IsType<T>(directive)) {
      auto candidate_block = dependencies.var_graph.at(var_idx).block_made;
      auto ancestor = dependencies.CommonBlockAncestor(candidate_block, current_block_idx);

      // todo: shift directives back for earlier ancestor blocks
      if (ancestor == current_block_idx || ancestor == candidate_block) {
        const auto* candidate = cotyl::unique_ptr_cast<const T>(directive);
        if (predicate(*candidate, op)) {
          replacement.emplace(op.idx, candidate->idx);
          return true;
        }
      }
    }
  }
  return false;
}


bool BasicOptimizer::ResolveBranchIndirection(calyx::Branch& op) {
  if (program.blocks.at(op.dest).size() == 1) {
    // single branch block
    // recursion resolves single branch chains
    auto& link_directive = program.blocks.at(op.dest)[0];
    if (IsType<UnconditionalBranch>(link_directive)) {
      auto* link = cotyl::unique_ptr_cast<UnconditionalBranch>(link_directive);
      dependencies.block_graph.at(link->dest).from.insert(current_block_idx);
      auto& to = dependencies.block_graph.at(current_block_idx).to;
      to.erase(op.dest);
      to.insert(link->dest);
      op.dest = link->dest;
      op.Emit(*this);
      return true;
    }
  }
  return false;
}

void BasicOptimizer::EmitProgram(Program& _program) {
  dependencies.EmitProgram(_program);
  new_program.functions    = std::move(_program.functions);
  new_program.globals      = std::move(_program.globals);
  new_program.strings      = std::move(_program.strings);

  for (const auto& [symbol, entry] : new_program.functions) {
    cotyl::unordered_set<block_label_t> visited = {};
    cotyl::unordered_set<block_label_t> todo = {entry};
    vars_found = {};

    while (!todo.empty()) {
      auto block = *todo.begin();
      todo.erase(todo.begin());

      auto inserted = new_program.blocks.emplace(block, calyx::Program::block_t{}).first;
      current_block = &inserted->second;
      current_block_idx = block;

      while (block) {
        for (const auto& directive : _program.blocks.at(block)) {
          directive->Emit(*this);
        }
        visited.insert(block);

        const auto& block_deps = dependencies.block_graph.at(block);
        if (block_deps.to.size() == 1) {
          auto next = *block_deps.to.begin();
          if (!visited.contains(next) && dependencies.block_graph.at(next).from.size() == 1) {
            // pop ending branch
            dependencies.block_graph.at(block).to = dependencies.block_graph.at(next).to;
            current_block->pop_back();
            block = next;
          }
          else {
            if (!visited.contains(next)) {
              todo.insert(next);
            }
            block = 0;
          }
        }
        else {
          for (const auto next : block_deps.to) {
            if (!visited.contains(next)) {
              todo.insert(next);
            }
          }
          block = 0;
        }
      }
    }
  }
}

void BasicOptimizer::Emit(AllocateLocal& op) {
  EmitNew<AllocateLocal>(op);
}

void BasicOptimizer::Emit(DeallocateLocal& op) {
  EmitNew<DeallocateLocal>(op);
}

template<typename To, typename From>
void BasicOptimizer::EmitCast(Cast<To, From>& op) {
  TryReplace(op.right_idx);
  auto replaced = FindExprResultReplacement(op, [&](auto& op, auto& candidate) {
    return candidate.right_idx == op.right_idx;
  });
  if (!replaced) {
    if constexpr(!std::is_same_v<To, Pointer> && !std::is_same_v<From, Pointer>) {
      auto [block, in_block] = vars_found.at(op.right_idx);
      auto& right_directive = new_program.blocks.at(block)[in_block];
      if (IsType<Imm<From>>(right_directive)) {
        auto* right_imm = cotyl::unique_ptr_cast<Imm<From>>(right_directive);
        auto repl = Imm<calyx_upcast_t<To>>(op.idx, (To)right_imm->value);
        Emit(repl);
        return;
      }
    }
    EmitExprCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitLoadLocal(LoadLocal<T>& op) {
  EmitExprCopy(op);
}

void BasicOptimizer::Emit(LoadLocalAddr& op) {
  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitStoreLocal(StoreLocal<T>& op) {
  TryReplace(op.src);
  EmitNew<StoreLocal<T>>(op);
}

template<typename T>
void BasicOptimizer::EmitLoadGlobal(LoadGlobal<T>& op) {
  EmitExprCopy(op);
}

void BasicOptimizer::Emit(LoadGlobalAddr& op) {
  auto replaced = FindExprResultReplacement(op, [&](auto& op, auto& candidate) {
    return candidate.symbol == op.symbol;
  });
  if (!replaced) {
    EmitExprCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitStoreGlobal(StoreGlobal<T>& op) {
  TryReplace(op.src);
  EmitNew<StoreGlobal<T>>(op);
}

template<typename T>
void BasicOptimizer::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  TryReplace(op.ptr_idx);
  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitStoreToPointer(StoreToPointer<T>& op) {
  TryReplace(op.src);
  TryReplace(op.ptr_idx);
  EmitNew<StoreToPointer<T>>(op);
}

template<typename T>
void BasicOptimizer::EmitCall(Call<T>& op) {
  TryReplace(op.fn_idx);
  for (auto& [var_idx, arg] : op.args) {
    TryReplace(var_idx);
  }
  for (auto& [var_idx, arg] : op.var_args) {
    TryReplace(var_idx);
  }

  vars_found[op.idx] = std::make_pair(current_block_idx, current_block->size());
  EmitNew<Call<T>>(op);
}

template<typename T>
void BasicOptimizer::EmitCallLabel(CallLabel<T>& op) {
  for (auto& [var_idx, arg] : op.args) {
    TryReplace(var_idx);
  }
  for (auto& [var_idx, arg] : op.var_args) {
    TryReplace(var_idx);
  }

  vars_found[op.idx] = std::make_pair(current_block_idx, current_block->size());
  EmitNew<CallLabel<T>>(op);
}

void BasicOptimizer::Emit(ArgMakeLocal& op) {
  EmitNew<ArgMakeLocal>(op);
}

template<typename T>
void BasicOptimizer::EmitReturn(Return<T>& op) {
  TryReplace(op.idx);
  EmitNew<Return<T>>(op);
}

template<typename T>
void BasicOptimizer::EmitImm(Imm<T>& op) {
  auto replaced = FindExprResultReplacement(op, [&](auto& op, auto& candidate) {
    return candidate.value == op.value;
  });
  if (!replaced) {
    EmitExprCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitUnop(Unop<T>& op) {
  TryReplace(op.right_idx);

  {
    auto [block, in_block] = vars_found.at(op.right_idx);
    auto& right_directive = new_program.blocks.at(block)[in_block];
    if (IsType<Unop<T>>(right_directive)) {
      const auto* right = cotyl::unique_ptr_cast<Unop<T>>(right_directive);
      if (right->op == op.op) {
        // ~ and - both cancel themselves out
        replacement[op.idx] = right->right_idx;
        return;
      }
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitBinop(Binop<T>& op) {
  TryReplace(op.left_idx);
  TryReplace(op.right_idx);

  // replace right side
  {
    auto [block, in_block] = vars_found.at(op.right_idx);
    auto& right_directive = new_program.blocks.at(block)[in_block];
    if (IsType<Imm<T>>(right_directive)) {
      T right_imm = cotyl::unique_ptr_cast<Imm<T>>(right_directive)->value;
      auto repl = BinopImm<T>(op.idx, op.left_idx, op.op, right_imm);
      Emit(repl);
      return;
    }
    else if (IsType<Unop<T>>(right_directive)) {
      const auto* right = cotyl::unique_ptr_cast<Unop<T>>(right_directive);
      if (right->op == UnopType::Neg) {
        if (op.op == BinopType::Add) {
          auto repl = Binop<T>(op.idx, op.left_idx, BinopType::Sub, right->right_idx);
          Emit(repl);
          return;
        }
        else if (op.op == BinopType::Sub) {
          auto repl = Binop<T>(op.idx, op.left_idx, BinopType::Add, right->right_idx);
          Emit(repl);
          return;
        }
      }
    }
  }

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  {
    auto [block, in_block] = vars_found.at(op.left_idx);
    auto& left_directive = new_program.blocks.at(block)[in_block];
    if (IsType<Imm<T>>(left_directive)) {
      T left_imm = cotyl::unique_ptr_cast<Imm<T>>(left_directive)->value;
      switch (op.op) {
        case BinopType::Add:
        case BinopType::BinAnd:
        case BinopType::BinOr:
        case BinopType::BinXor:
        case BinopType::Mul: {
          auto repl = BinopImm<T>(op.idx, op.right_idx, op.op, left_imm);
          Emit(repl);
          return;
        }
        case BinopType::Sub: {
          auto repl = BinopImm<T>(op.idx, -left_imm, BinopType::Add, op.right_idx);
          Emit(repl);
          return;
        }
        case BinopType::Div:
        case BinopType::Mod:
          // non-commutative
          break;
      }
    }
    else if (IsType<Unop<T>>(left_directive)) {
      const auto* left = cotyl::unique_ptr_cast<Unop<T>>(left_directive);
      if (left->op == UnopType::Neg) {
        if (op.op == BinopType::Add) {
          auto repl = Binop<T>(op.idx, op.right_idx, BinopType::Sub, left->right_idx);
          Emit(repl);
          return;
        }
      }
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitBinopImm(BinopImm<T>& op) {
  TryReplace(op.left_idx);

  switch (op.op) {
    case BinopType::Add:
    case BinopType::Sub:
    case BinopType::BinOr:
    case BinopType::BinXor:
      if (op.right == 0) {
        replacement[op.idx] = op.left_idx;
        return;
      }
      break;
    case BinopType::Mul:
      if (op.right == 1) {
        replacement[op.idx] = op.left_idx;
        return;
      }
      else if (op.right == 0) {
        auto repl = Imm<T>(op.idx, 0);
        Emit(repl);
        return;
      }
      break;
    case BinopType::Div:
      if (op.right == 1) {
        replacement[op.idx] = op.left_idx;
        return;
      }
    case BinopType::Mod:
      break;
    case BinopType::BinAnd:
      if (op.right == 0) {
        auto repl = Imm<T>(op.idx, 0);
        Emit(repl);
        return;
      }
      break;
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitShift(Shift<T>& op) {
  TryReplace(op.left_idx);
  TryReplace(op.right_idx);

  // replace right side
  {
    auto [block, in_block] = vars_found.at(op.right_idx);
    auto& right_directive = new_program.blocks.at(block)[in_block];
    if (IsType<Imm<u32>>(right_directive)) {
      u32 right_imm = cotyl::unique_ptr_cast<Imm<u32>>(right_directive)->value;
      auto repl = ShiftImm<T>(op.idx, op.left_idx, op.op, right_imm);
      Emit(repl);
      return;
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitShiftImm(ShiftImm<T>& op) {
  TryReplace(op.left_idx);

  if (op.right == 0) {
    replacement[op.idx] = op.left_idx;
    return;
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitCompare(Compare<T>& op) {
  TryReplace(op.left_idx);
  TryReplace(op.right_idx);


  // replace right side
  {
    auto [block, in_block] = vars_found.at(op.right_idx);
    auto& right_directive = new_program.blocks.at(block)[in_block];
    if (IsType<Imm<T>>(right_directive)) {
      T right_imm = cotyl::unique_ptr_cast<Imm<T>>(right_directive)->value;
      auto repl = CompareImm<T>(op.idx, op.left_idx, op.op, right_imm);
      Emit(repl);
      return;
    }
  }

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  {
    auto [block, in_block] = vars_found.at(op.left_idx);
    auto& left_directive = new_program.blocks.at(block)[in_block];
    if (IsType<Imm<T>>(left_directive)) {
      T left_imm = cotyl::unique_ptr_cast<Imm<T>>(left_directive)->value;
      CmpType flipped = op.op;

      switch (op.op) {
        case CmpType::Eq:
        case CmpType::Ne: break;
        case CmpType::Lt: flipped = CmpType::Gt; break;
        case CmpType::Le: flipped = CmpType::Ge; break;
        case CmpType::Gt: flipped = CmpType::Lt; break;
        case CmpType::Ge: flipped = CmpType::Le; break;
      }
      auto repl = CompareImm<T>(op.idx, op.right_idx, flipped, left_imm);
      Emit(repl);
      return;
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitCompareImm(CompareImm<T>& op) {
  TryReplace(op.left_idx);
  EmitExprCopy(op);
}

void BasicOptimizer::Emit(UnconditionalBranch& op) {
  if (!ResolveBranchIndirection(op)) {
    EmitNew<UnconditionalBranch>(op);
  }
}

template<typename T>
void BasicOptimizer::EmitBranchCompare(BranchCompare<T>& op) {
  TryReplace(op.left_idx);
  TryReplace(op.right_idx);

  // replace right side
  {
    auto [block, in_block] = vars_found.at(op.right_idx);
    auto& right_directive = new_program.blocks.at(block)[in_block];
    if (IsType<Imm<T>>(right_directive)) {
      T right_imm = cotyl::unique_ptr_cast<Imm<T>>(right_directive)->value;
      auto repl = BranchCompareImm<T>(op.dest, op.left_idx, op.op, right_imm);
      Emit(repl);
      return;
    }
  }

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  {
    auto [block, in_block] = vars_found.at(op.left_idx);
    auto& left_directive = new_program.blocks.at(block)[in_block];
    if (IsType<Imm<T>>(left_directive)) {
      T left_imm = cotyl::unique_ptr_cast<Imm<T>>(left_directive)->value;
      CmpType flipped = op.op;

      switch (op.op) {
        case CmpType::Eq:
        case CmpType::Ne: break;
        case CmpType::Lt: flipped = CmpType::Gt; break;
        case CmpType::Le: flipped = CmpType::Ge; break;
        case CmpType::Gt: flipped = CmpType::Lt; break;
        case CmpType::Ge: flipped = CmpType::Le; break;
      }
      auto repl = BranchCompareImm<T>(op.dest, op.right_idx, flipped, left_imm);
      Emit(repl);
      return;
    }
  }

  if (!ResolveBranchIndirection(op)) {
    EmitNew<BranchCompare<T>>(op);
  }
}

template<typename T>
void BasicOptimizer::EmitBranchCompareImm(BranchCompareImm<T>& op) {
  TryReplace(op.left_idx);

  if constexpr(!std::is_same_v<T, Pointer>) {
    auto [block, in_block] = vars_found.at(op.left_idx);
    auto& left_directive = new_program.blocks.at(block)[in_block];
    if (IsType<Imm<T>>(left_directive)) {
      T left_imm = cotyl::unique_ptr_cast<Imm<T>>(left_directive)->value;
      CmpType flipped = op.op;

      auto to_unconditional = [&] {
        auto repl = UnconditionalBranch(op.dest);
        Emit(repl);
      };

      switch (op.op) {
        case CmpType::Eq: if (left_imm == op.right) to_unconditional(); break;
        case CmpType::Ne: if (left_imm != op.right) to_unconditional(); break;
        case CmpType::Lt: if (left_imm <  op.right) to_unconditional(); break;
        case CmpType::Le: if (left_imm <= op.right) to_unconditional(); break;
        case CmpType::Gt: if (left_imm >  op.right) to_unconditional(); break;
        case CmpType::Ge: if (left_imm >= op.right) to_unconditional(); break;
      }
      return;
    }
  }

  if (!ResolveBranchIndirection(op)) {
    EmitNew<BranchCompareImm<T>>(op);
  }
}

void BasicOptimizer::Emit(Select& op) {
  TryReplace(op.idx);
  EmitNew<Select>(op);
}

template<typename T>
void BasicOptimizer::EmitAddToPointer(AddToPointer<T>& op) {
  TryReplace(op.ptr_idx);
  TryReplace(op.right_idx);

  auto [block, in_block] = vars_found.at(op.right_idx);
  auto& right_directive = new_program.blocks.at(block)[in_block];
  if (IsType<Imm<T>>(right_directive)) {
    T right_imm = cotyl::unique_ptr_cast<Imm<T>>(right_directive)->value;
    if (right_imm == 0) {
      replacement[op.idx] = op.ptr_idx;
    }
    else if (op.op == PtrAddType::Add) {
      auto repl = AddToPointerImm(op.idx, op.ptr_idx, op.stride, right_imm);
      Emit(repl);
    }
    else {
      auto repl = AddToPointerImm(op.idx, op.ptr_idx, op.stride, -right_imm);
      Emit(repl);
    }
    return;
  }
  EmitExprCopy(op);
}

void BasicOptimizer::Emit(AddToPointerImm& op) {
  TryReplace(op.ptr_idx);
  if (op.right == 0) {
    replacement[op.idx] = op.ptr_idx;
    return;
  }
  EmitExprCopy(op);
}

#define BACKEND_NAME BasicOptimizer
#include "calyx/backend/Templates.inl"

}