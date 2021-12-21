#include "CommonExpressions.h"
#include "IRCompare.h"
#include "Cast.h"


namespace epi {

void CommonExpressions::TryReplace(calyx::var_index_t& var_idx) const {
  if (replacement.contains(var_idx)) {
    var_idx = replacement.at(var_idx);
  }
}

calyx::block_label_t CommonExpressions::CommonBlockAncestor(calyx::block_label_t first, calyx::block_label_t second) const {
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

std::set<calyx::block_label_t> CommonExpressions::UpwardClosure(calyx::block_label_t base) const {
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
bool CommonExpressions::ReplaceIf(T& op, F predicate) {
  for (const auto& [var_idx, loc] : vars_found) {
    auto& directive = new_program.blocks.at(loc.first)[loc.second];
    if (IsType<T>(directive)) {
      const auto* candidate = cotyl::unique_ptr_cast<const T>(directive);
      if (predicate(*candidate, op)) {
        replacement.emplace(op.idx, candidate->idx);
        return true;
      }
    }
  }
  return false;
}

void CommonExpressions::EmitProgram(Program& program) {
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

void CommonExpressions::Emit(AllocateLocal& op) {
  EmitNew(std::make_unique<AllocateLocal>(op));
}

void CommonExpressions::Emit(DeallocateLocal& op) {
  EmitNew(std::make_unique<DeallocateLocal>(op));
}

template<typename To, typename From>
void CommonExpressions::EmitCast(Cast<To, From>& op) {
  TryReplace(op.right_idx);
  auto replaced = ReplaceIf(op, [&](auto& op, auto& candidate) {
    return candidate.right_idx == op.right_idx;
  });
  if (!replaced) {
    vars_found.emplace(op.idx, EmitNew(std::make_unique<Cast<To, From>>(op)));
  }
}

template<typename T>
void CommonExpressions::EmitLoadLocal(LoadLocal<T>& op) {
  EmitNew(std::make_unique<LoadLocal<T>>(op));
}

void CommonExpressions::Emit(LoadLocalAddr& op) {
  EmitNew(std::make_unique<LoadLocalAddr>(op));
}

template<typename T>
void CommonExpressions::EmitStoreLocal(StoreLocal<T>& op) {
  EmitNew(std::make_unique<StoreLocal<T>>(op));
}

template<typename T>
void CommonExpressions::EmitLoadGlobal(LoadGlobal<T>& op) {
  EmitNew(std::make_unique<LoadGlobal<T>>(op));
}

void CommonExpressions::Emit(LoadGlobalAddr& op) {
  auto replaced = ReplaceIf(op, [&](auto& op, auto& candidate) {
    return candidate.symbol == op.symbol;
  });
  if (!replaced) {
    vars_found.emplace(op.idx, EmitNew(std::make_unique<LoadGlobalAddr>(op)));
  }
}

template<typename T>
void CommonExpressions::EmitStoreGlobal(StoreGlobal<T>& op) {
  EmitNew(std::make_unique<StoreGlobal<T>>(op));
}

template<typename T>
void CommonExpressions::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  EmitNew(std::make_unique<LoadFromPointer<T>>(op));
}

template<typename T>
void CommonExpressions::EmitStoreToPointer(StoreToPointer<T>& op) {
  EmitNew(std::make_unique<StoreToPointer<T>>(op));
}

template<typename T>
void CommonExpressions::EmitCall(Call<T>& op) {
  TryReplace(op.fn_idx);
  EmitNew(std::make_unique<Call<T>>(op));
}

template<typename T>
void CommonExpressions::EmitCallLabel(CallLabel<T>& op) {
  EmitNew(std::make_unique<CallLabel<T>>(op));
}

void CommonExpressions::Emit(ArgMakeLocal& op) {
  EmitNew(std::make_unique<ArgMakeLocal>(op));
}

template<typename T>
void CommonExpressions::EmitReturn(Return<T>& op) {
  EmitNew(std::make_unique<Return<T>>(op));
}

template<typename T>
void CommonExpressions::EmitImm(Imm<T>& op) {
  EmitNew(std::make_unique<Imm<T>>(op));
}

template<typename T>
void CommonExpressions::EmitUnop(Unop<T>& op) {
  EmitNew(std::make_unique<Unop<T>>(op));
}

template<typename T>
void CommonExpressions::EmitBinop(Binop<T>& op) {
  EmitNew(std::make_unique<Binop<T>>(op));
}

template<typename T>
void CommonExpressions::EmitBinopImm(BinopImm<T>& op) {
  EmitNew(std::make_unique<BinopImm<T>>(op));
}

template<typename T>
void CommonExpressions::EmitShift(Shift<T>& op) {
  EmitNew(std::make_unique<Shift<T>>(op));
}

template<typename T>
void CommonExpressions::EmitShiftImm(ShiftImm<T>& op) {
  EmitNew(std::make_unique<ShiftImm<T>>(op));
}

template<typename T>
void CommonExpressions::EmitCompare(Compare<T>& op) {
  EmitNew(std::make_unique<Compare<T>>(op));
}

template<typename T>
void CommonExpressions::EmitCompareImm(CompareImm<T>& op) {
  EmitNew(std::make_unique<CompareImm<T>>(op));
}

void CommonExpressions::Emit(UnconditionalBranch& op) {
  EmitNew(std::make_unique<UnconditionalBranch>(op));
}

template<typename T>
void CommonExpressions::EmitBranchCompare(BranchCompare<T>& op) {
  EmitNew(std::make_unique<BranchCompare<T>>(op));
}

template<typename T>
void CommonExpressions::EmitBranchCompareImm(BranchCompareImm<T>& op) {
  EmitNew(std::make_unique<BranchCompareImm<T>>(op));
}

void CommonExpressions::Emit(Select& op) {
  EmitNew(std::make_unique<Select>(op));
}

template<typename T>
void CommonExpressions::EmitAddToPointer(AddToPointer<T>& op) {
  EmitNew(std::make_unique<AddToPointer<T>>(op));
}

void CommonExpressions::Emit(AddToPointerImm& op) {
  EmitNew(std::make_unique<AddToPointerImm>(op));
}

#define BACKEND_NAME CommonExpressions
#include "calyx/backend/Templates.inl"

}