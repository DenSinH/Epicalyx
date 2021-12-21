#include "BasicOptimizer.h"
#include "IRCompare.h"
#include "Cast.h"


namespace epi {

void BasicOptimizer::TryReplace(calyx::var_index_t& var_idx) const {
  if (replacement.contains(var_idx)) {
    var_idx = replacement.at(var_idx);
  }
}

calyx::block_label_t BasicOptimizer::CommonBlockAncestor(calyx::block_label_t first, calyx::block_label_t second) const {
  std::set<calyx::block_label_t> ancestors{first, second};

  // we use the fact that if block1 > block2 then block1 can never be an ancestor of block2
  while (ancestors.size() > 1) {
    auto max_ancestor = *ancestors.rbegin();
    ancestors.erase(std::prev(ancestors.end()));
    if (!dependencies.block_graph.contains(max_ancestor)) {
      return 0;
    }
    auto& deps = dependencies.block_graph.at(max_ancestor);
    if (deps.from.empty()) {
      return 0;
    }
    for (auto dep : deps.from) {
      ancestors.insert(dep);
    }
  }

  // at this point only one ancestor should be left
  return *ancestors.begin();
}

std::set<calyx::block_label_t> BasicOptimizer::UpwardClosure(calyx::block_label_t base) const {
  std::unordered_set<calyx::block_label_t> closure{base};
  std::unordered_set<calyx::block_label_t> search{base};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    if (dependencies.block_graph.contains(current)) {
      for (const auto& dep : dependencies.block_graph.at(current).to) {
        if (!closure.contains(dep)) {
          closure.emplace(dep);
          search.emplace(dep);
        }
      }
    }
  }

  return {closure.begin(), closure.end()};
}

template<typename T, class F>
bool BasicOptimizer::ReplaceIf(T& op, F predicate) {
  for (const auto& [var_idx, loc] : vars_found) {
    auto& directive = new_program.blocks.at(loc.first)[loc.second];
    if (IsType<T>(directive)) {
      auto candidate_block = dependencies.var_graph.at(var_idx).block_made;
      auto ancestor = CommonBlockAncestor(candidate_block, current_block_idx);

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

void BasicOptimizer::EmitProgram(Program& program) {
  dependencies.EmitProgram(program);
  new_program.functions    = std::move(program.functions);
  new_program.globals      = std::move(program.globals);
  new_program.global_init  = std::move(program.global_init);
  new_program.local_labels = std::move(program.local_labels);
  new_program.strings      = std::move(program.strings);


  // copy over global initializer blocks
  for (const auto& [symbol, global_init] : new_program.global_init) {
    if (std::holds_alternative<calyx::block_label_t>(global_init)) {
      auto closure = UpwardClosure(std::get<calyx::block_label_t>(global_init));

      for (auto block : closure) {
        new_program.blocks.emplace(block, std::move(program.blocks[block]));
        program.blocks.erase(block);
      }
    }
  }

  for (const auto& [symbol, entry] : new_program.functions) {
    auto closure = UpwardClosure(entry);
    vars_found = {};

    for (const auto& block : closure) {
      auto inserted = new_program.blocks.emplace(block, calyx::Program::block_t{}).first;
      current_block = &inserted->second;
      current_block_idx = block;
      for (const auto& directive : program.blocks.at(block)) {
        directive->Emit(*this);
      }
    }
  }
}

void BasicOptimizer::Emit(AllocateLocal& op) {
  EmitNew(std::make_unique<AllocateLocal>(op));
}

void BasicOptimizer::Emit(DeallocateLocal& op) {
  EmitNew(std::make_unique<DeallocateLocal>(op));
}

template<typename To, typename From>
void BasicOptimizer::EmitCast(Cast<To, From>& op) {
  TryReplace(op.right_idx);
  auto replaced = ReplaceIf(op, [&](auto& op, auto& candidate) {
    return candidate.right_idx == op.right_idx;
  });
  if (!replaced) {
    if constexpr(!std::is_same_v<To, Pointer> && !std::is_same_v<From, Pointer>) {
      auto [block, in_block] = vars_found.at(op.right_idx);
      auto& right_directive = new_program.blocks.at(block)[in_block];
      if (IsType<Imm<From>>(right_directive)) {
        auto* right_imm = cotyl::unique_ptr_cast<Imm<From>>(right_directive);
        vars_found.emplace(op.idx, EmitNew(std::make_unique<Imm<calyx_upcast_t<To>>>(op.idx, (To)right_imm->value)));
        return;
      }
    }
    vars_found.emplace(op.idx, EmitNew(std::make_unique<Cast<To, From>>(op)));
  }
}

template<typename T>
void BasicOptimizer::EmitLoadLocal(LoadLocal<T>& op) {
  EmitNew(std::make_unique<LoadLocal<T>>(op));
}

void BasicOptimizer::Emit(LoadLocalAddr& op) {
  EmitNew(std::make_unique<LoadLocalAddr>(op));
}

template<typename T>
void BasicOptimizer::EmitStoreLocal(StoreLocal<T>& op) {
  TryReplace(op.src);
  EmitNew(std::make_unique<StoreLocal<T>>(op));
}

template<typename T>
void BasicOptimizer::EmitLoadGlobal(LoadGlobal<T>& op) {
  EmitNew(std::make_unique<LoadGlobal<T>>(op));
}

void BasicOptimizer::Emit(LoadGlobalAddr& op) {
  auto replaced = ReplaceIf(op, [&](auto& op, auto& candidate) {
    return candidate.symbol == op.symbol;
  });
  if (!replaced) {
    vars_found.emplace(op.idx, EmitNew(std::make_unique<LoadGlobalAddr>(op)));
  }
}

template<typename T>
void BasicOptimizer::EmitStoreGlobal(StoreGlobal<T>& op) {
  TryReplace(op.src);
  EmitNew(std::make_unique<StoreGlobal<T>>(op));
}

template<typename T>
void BasicOptimizer::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  TryReplace(op.ptr_idx);
  EmitNew(std::make_unique<LoadFromPointer<T>>(op));
}

template<typename T>
void BasicOptimizer::EmitStoreToPointer(StoreToPointer<T>& op) {
  TryReplace(op.src);
  TryReplace(op.ptr_idx);
  EmitNew(std::make_unique<StoreToPointer<T>>(op));
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

  EmitNew(std::make_unique<Call<T>>(op));
}

template<typename T>
void BasicOptimizer::EmitCallLabel(CallLabel<T>& op) {
  for (auto& [var_idx, arg] : op.args) {
    TryReplace(var_idx);
  }
  for (auto& [var_idx, arg] : op.var_args) {
    TryReplace(var_idx);
  }

  EmitNew(std::make_unique<CallLabel<T>>(op));
}

void BasicOptimizer::Emit(ArgMakeLocal& op) {
  EmitNew(std::make_unique<ArgMakeLocal>(op));
}

template<typename T>
void BasicOptimizer::EmitReturn(Return<T>& op) {
  TryReplace(op.idx);
  EmitNew(std::make_unique<Return<T>>(op));
}

template<typename T>
void BasicOptimizer::EmitImm(Imm<T>& op) {
  TryReplace(op.idx);
  auto replaced = ReplaceIf(op, [&](auto& op, auto& candidate) {
    return candidate.value == op.value;
  });
  if (!replaced) {
    vars_found.emplace(op.idx, EmitNew(std::make_unique<Imm<T>>(op)));
  }
}

template<typename T>
void BasicOptimizer::EmitUnop(Unop<T>& op) {
  TryReplace(op.right_idx);
  EmitNew(std::make_unique<Unop<T>>(op));
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
      vars_found.emplace(op.idx, EmitNew(std::make_unique<BinopImm<T>>(op.idx, op.left_idx, op.op, right_imm)));
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
      switch (op.op) {
        case BinopType::Add:
        case BinopType::BinAnd:
        case BinopType::BinOr:
        case BinopType::BinXor:
        case BinopType::Mul:
          vars_found.emplace(op.idx, EmitNew(std::make_unique<BinopImm<T>>(op.idx, op.right_idx, op.op, left_imm)));
          return;
        case BinopType::Sub:
          vars_found.emplace(op.idx, EmitNew(std::make_unique<BinopImm<T>>(op.idx, -left_imm, BinopType::Add, op.right_idx)));
          return;
        case BinopType::Div:
        case BinopType::Mod:
          // non-commutative
          break;
      }
    }
  }

  EmitNew(std::make_unique<Binop<T>>(op));
}

template<typename T>
void BasicOptimizer::EmitBinopImm(BinopImm<T>& op) {
  TryReplace(op.left_idx);
  EmitNew(std::make_unique<BinopImm<T>>(op));
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
      vars_found.emplace(op.idx, EmitNew(std::make_unique<ShiftImm<T>>(op.idx, op.left_idx, op.op, right_imm)));
      return;
    }
  }

  EmitNew(std::make_unique<Shift<T>>(op));
}

template<typename T>
void BasicOptimizer::EmitShiftImm(ShiftImm<T>& op) {
  TryReplace(op.left_idx);
  EmitNew(std::make_unique<ShiftImm<T>>(op));
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
      vars_found.emplace(op.idx, EmitNew(std::make_unique<CompareImm<T>>(op.idx, op.left_idx, op.op, right_imm)));
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
      CmpType flipped;

      switch (op.op) {
        case CmpType::Eq:
        case CmpType::Ne: break;
        case CmpType::Lt: flipped = CmpType::Gt; break;
        case CmpType::Le: flipped = CmpType::Ge; break;
        case CmpType::Gt: flipped = CmpType::Lt; break;
        case CmpType::Ge: flipped = CmpType::Le; break;
      }
      vars_found.emplace(op.idx, EmitNew(std::make_unique<CompareImm<T>>(op.idx, op.right_idx, flipped, left_imm)));
    }
  }

  EmitNew(std::make_unique<Compare<T>>(op));
}

template<typename T>
void BasicOptimizer::EmitCompareImm(CompareImm<T>& op) {
  TryReplace(op.left_idx);
  EmitNew(std::make_unique<CompareImm<T>>(op));
}

void BasicOptimizer::Emit(UnconditionalBranch& op) {
  EmitNew(std::make_unique<UnconditionalBranch>(op));
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
      EmitNew(std::make_unique<BranchCompareImm<T>>(op.dest, op.left_idx, op.op, right_imm));
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
      CmpType flipped;

      switch (op.op) {
        case CmpType::Eq:
        case CmpType::Ne: break;
        case CmpType::Lt: flipped = CmpType::Gt; break;
        case CmpType::Le: flipped = CmpType::Ge; break;
        case CmpType::Gt: flipped = CmpType::Lt; break;
        case CmpType::Ge: flipped = CmpType::Le; break;
      }
      EmitNew(std::make_unique<BranchCompareImm<T>>(op.dest, op.right_idx, flipped, left_imm));
    }
  }
  EmitNew(std::make_unique<BranchCompare<T>>(op));
}

template<typename T>
void BasicOptimizer::EmitBranchCompareImm(BranchCompareImm<T>& op) {
  TryReplace(op.left_idx);
  EmitNew(std::make_unique<BranchCompareImm<T>>(op));
}

void BasicOptimizer::Emit(Select& op) {
  TryReplace(op.idx);
  EmitNew(std::make_unique<Select>(op));
}

template<typename T>
void BasicOptimizer::EmitAddToPointer(AddToPointer<T>& op) {
  TryReplace(op.ptr_idx);
  TryReplace(op.right_idx);
  EmitNew(std::make_unique<AddToPointer<T>>(op));
}

void BasicOptimizer::Emit(AddToPointerImm& op) {
  TryReplace(op.ptr_idx);
  EmitNew(std::make_unique<AddToPointerImm>(op));
}

#define BACKEND_NAME BasicOptimizer
#include "calyx/backend/Templates.inl"

}