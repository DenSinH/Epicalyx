#include "BasicOptimizer.h"
#include "IRCompare.h"
#include "Cast.h"
#include "Is.h"
#include "Containers.h"
#include "CustomAssert.h"

#include <iostream>


namespace epi {

void BasicOptimizer::FlushLocal(var_index_t loc_idx, Local&& local) {
  current_block->emplace_back(std::move(local.store));
  locals.erase(loc_idx);
}

void BasicOptimizer::FlushCurrentLocals() {
  for (auto& [loc_idx, local] : locals) {
    current_block->emplace_back(std::move(local.store));
  }
  locals = {};
}

void BasicOptimizer::TryReplaceVar(calyx::var_index_t& var_idx) const {
  if (var_replacement.contains(var_idx)) {
    var_idx = var_replacement.at(var_idx);
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
          var_replacement[op.idx] = candidate->idx;
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

template<typename T>
T* BasicOptimizer::TryGetVarDirective(var_index_t idx) {
  auto [block, in_block] = vars_found.at(idx);
  auto& directive = new_program.blocks.at(block)[in_block];
  if (IsType<T>(directive)) {
    return cotyl::unique_ptr_cast<T>(directive);
  }
  return nullptr;
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
      cotyl::Assert(locals.empty());

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
  EmitCopy(op);
}

void BasicOptimizer::Emit(DeallocateLocal& op) {
  EmitCopy(op);
}

template<typename To, typename From>
void BasicOptimizer::EmitCast(Cast<To, From>& op) {
  if constexpr(std::is_same_v<To, From>) {
    var_replacement[op.idx] = op.right_idx;
    return;
  }
  else {
    TryReplaceVar(op.right_idx);
    auto replaced = FindExprResultReplacement(op, [&](auto& op, auto& candidate) {
      return candidate.right_idx == op.right_idx;
    });
    if (!replaced) {
      auto* right_directive = TryGetVarDirective<Imm<From>>(op.right_idx);
      if constexpr(std::is_same_v<From, Pointer>) {
        if (right_directive) {
          auto repl = Imm<calyx_upcast_t<To>>(op.idx, (To)right_directive->value.value);
          Emit(repl);
          return;
        }
      }
      else if constexpr(std::is_same_v<To, Pointer>) {
        // todo: pointer immediates
      }
      else{
        if (right_directive) {
          auto repl = Imm<calyx_upcast_t<To>>(op.idx, (To)right_directive->value);
          Emit(repl);
          return;
        }
      }
      EmitExprCopy(op);
    }
  }
}

template<typename T>
void BasicOptimizer::EmitLoadLocal(LoadLocal<T>& op) {
  if (locals.contains(op.loc_idx)) {
    auto& repl = locals.at(op.loc_idx).replacement;
    repl->idx = op.idx;
    repl->Emit(*this);
  }
  else {
    EmitExprCopy(op);
  }
}

void BasicOptimizer::Emit(LoadLocalAddr& op) {
  var_aliases[op.idx] = op.loc_idx;
  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitStoreLocal(StoreLocal<T>& op) {
  TryReplaceVar(op.src);
  if constexpr(!std::is_same_v<T, Struct>) {
    var_index_t aliases = 0;
    if (var_aliases.contains(op.src)) {
      aliases = var_aliases.at(op.src);
    }
    locals[op.loc_idx] = Local{
            .aliases = aliases,
            .replacement = std::make_unique<Cast<T, calyx_upcast_t<T>>>(0, op.src),
            .store = std::make_unique<StoreLocal<T>>(op)
    };
  }
  else {
    EmitCopy(op);
  }
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
  TryReplaceVar(op.src);
  EmitCopy(op);
}

template<typename T>
void BasicOptimizer::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  TryReplaceVar(op.ptr_idx);
  if (var_aliases.contains(op.ptr_idx)) {
    auto alias = var_aliases.at(op.ptr_idx);
    if (locals.contains(alias) && op.offset == 0) {
      // emit aliased local replacement
      auto& repl = locals.at(alias).replacement;
      repl->idx = op.idx;
      repl->Emit(*this);
    }
    else {
      // load aliased local directly
      auto repl = LoadLocal<T>(op.idx, alias, op.offset);
    }
    return;
  }
  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitStoreToPointer(StoreToPointer<T>& op) {
  TryReplaceVar(op.src);
  TryReplaceVar(op.ptr_idx);
  if constexpr(!std::is_same_v<T, Struct>) {
    if (var_aliases.contains(op.ptr_idx) && op.offset == 0) {
      auto alias = var_aliases.at(op.ptr_idx);

      // storing aliased variable to another alias
      // i.e. int* zalias1 = &z; int* zalias2 = zalias1;
      var_index_t aliases = 0;
      if (var_aliases.contains(op.src)) {
        aliases = var_aliases.at(op.src);
      }

      // replace alias
      locals[alias] = Local{
              .aliases = aliases,
              .replacement = std::make_unique<Cast<T, calyx_upcast_t<T>>>(0, op.src),
              .store = std::make_unique<StoreLocal<T>>(alias, op.src)
      };
      return;
    }
  }
  EmitCopy(op);
}

template<typename T>
void BasicOptimizer::EmitCall(Call<T>& op) {
  TryReplaceVar(op.fn_idx);
  for (auto& [var_idx, arg] : op.args) {
    TryReplaceVar(var_idx);
  }
  for (auto& [var_idx, arg] : op.var_args) {
    TryReplaceVar(var_idx);
  }

  vars_found[op.idx] = std::make_pair(current_block_idx, current_block->size());
  EmitCopy(op);
}

template<typename T>
void BasicOptimizer::EmitCallLabel(CallLabel<T>& op) {
  for (auto& [var_idx, arg] : op.args) {
    TryReplaceVar(var_idx);
  }
  for (auto& [var_idx, arg] : op.var_args) {
    TryReplaceVar(var_idx);
  }

  vars_found[op.idx] = std::make_pair(current_block_idx, current_block->size());
  EmitCopy(op);
}

void BasicOptimizer::Emit(ArgMakeLocal& op) {
  EmitCopy(op);
}

template<typename T>
void BasicOptimizer::EmitReturn(Return<T>& op) {
  TryReplaceVar(op.idx);
  // no need to flush locals right before a return
  locals = {};
  EmitCopy(op);
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
  TryReplaceVar(op.right_idx);

  {
    auto* right_unop = TryGetVarDirective<Unop<T>>(op.right_idx);
    if (right_unop) {
      if (right_unop->op == op.op) {
        // ~ and - both cancel themselves out
        var_replacement[op.idx] = right_unop->right_idx;
        return;
      }
    }
    auto* right_imm = TryGetVarDirective<Imm<T>>(op.right_idx);
    if (right_imm) {
      T result;
      switch (op.op) {
        case UnopType::Neg: result = -right_imm->value; break;
        case UnopType::BinNot: if constexpr(calyx::is_calyx_integral_type_v<T>) result = ~right_imm->value; break;
          break;
      }
      auto repl = Imm<T>(op.idx, result);
      Emit(repl);
      return;
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitBinop(Binop<T>& op) {
  TryReplaceVar(op.left_idx);
  TryReplaceVar(op.right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<T>>(op.right_idx);
    if (right_imm) {
      auto repl = BinopImm<T>(op.idx, op.left_idx, op.op, right_imm->value);
      Emit(repl);
      return;
    }
    auto* right_unop = TryGetVarDirective<Unop<T>>(op.right_idx);
    if (right_unop) {
      if (right_unop->op == UnopType::Neg) {
        if (op.op == BinopType::Add) {
          auto repl = Binop<T>(op.idx, op.left_idx, BinopType::Sub, right_unop->right_idx);
          Emit(repl);
          return;
        }
        else if (op.op == BinopType::Sub) {
          auto repl = Binop<T>(op.idx, op.left_idx, BinopType::Add, right_unop->right_idx);
          Emit(repl);
          return;
        }
      }
    }
  }

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      switch (op.op) {
        case BinopType::Add:
        case BinopType::BinAnd:
        case BinopType::BinOr:
        case BinopType::BinXor:
        case BinopType::Mul: {
          auto repl = BinopImm<T>(op.idx, op.right_idx, op.op, left_imm->value);
          Emit(repl);
          return;
        }
        case BinopType::Sub: {
          auto repl = BinopImm<T>(op.idx, -left_imm->value, BinopType::Add, op.right_idx);
          Emit(repl);
          return;
        }
        case BinopType::Div:
        case BinopType::Mod:
          // non-commutative
          break;
      }
    }

    auto* left_unop = TryGetVarDirective<Unop<T>>(op.left_idx);
    if (left_unop) {
      if (left_unop->op == UnopType::Neg) {
        if (op.op == BinopType::Add) {
          auto repl = Binop<T>(op.idx, op.right_idx, BinopType::Sub, left_unop->right_idx);
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
  TryReplaceVar(op.left_idx);

  switch (op.op) {
    case BinopType::Add:
    case BinopType::Sub:
    case BinopType::BinOr:
    case BinopType::BinXor:
      if (op.right == 0) {
        var_replacement[op.idx] = op.left_idx;
        return;
      }
      break;
    case BinopType::Mul:
      if (op.right == 1) {
        var_replacement[op.idx] = op.left_idx;
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
        var_replacement[op.idx] = op.left_idx;
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

  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      T result;
      switch (op.op) {
        case BinopType::Add: result = left_imm->value + op.right; break;
        case BinopType::Sub: result = left_imm->value - op.right; break;
        case BinopType::Mul: result = left_imm->value * op.right; break;
        case BinopType::Div: result = left_imm->value / op.right; break;
        default:
          if constexpr(is_calyx_integral_type_v<T>) {
            switch (op.op) {
              case BinopType::Mod: result = left_imm->value % op.right; break;
              case BinopType::BinAnd: result = left_imm->value & op.right; break;
              case BinopType::BinOr: result = left_imm->value | op.right; break;
              case BinopType::BinXor: result = left_imm->value ^ op.right; break;
              default: break;
            }
          }
          break;
      }
      auto repl = Imm<T>(op.idx, result);
      Emit(repl);
      return;
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitShift(Shift<T>& op) {
  TryReplaceVar(op.left_idx);
  TryReplaceVar(op.right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<u32>>(op.right_idx);
    if (right_imm) {
      auto repl = ShiftImm<T>(op.idx, op.left_idx, op.op, right_imm->value);
      Emit(repl);
      return;
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitShiftImm(ShiftImm<T>& op) {
  TryReplaceVar(op.left_idx);

  if (op.right == 0) {
    var_replacement[op.idx] = op.left_idx;
    return;
  }

  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      T result;
      switch (op.op) {
        case ShiftType::Left: result = left_imm->value << op.right; break;
        case ShiftType::Right: result = left_imm->value >> op.right; break;
      }

      auto repl = Imm<T>(op.idx, result);
      Emit(repl);
      return;
    }
  }
  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitCompare(Compare<T>& op) {
  TryReplaceVar(op.left_idx);
  TryReplaceVar(op.right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<T>>(op.right_idx);
    if (right_imm) {
      auto repl = CompareImm<T>(op.idx, op.left_idx, op.op, right_imm->value);
      Emit(repl);
      return;
    }
  }

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      CmpType flipped = op.op;

      switch (op.op) {
        case CmpType::Eq:
        case CmpType::Ne: break;
        case CmpType::Lt: flipped = CmpType::Gt; break;
        case CmpType::Le: flipped = CmpType::Ge; break;
        case CmpType::Gt: flipped = CmpType::Lt; break;
        case CmpType::Ge: flipped = CmpType::Le; break;
      }
      auto repl = CompareImm<T>(op.idx, op.right_idx, flipped, left_imm->value);
      Emit(repl);
      return;
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitCompareImm(CompareImm<T>& op) {
  TryReplaceVar(op.left_idx);
  FlushCurrentLocals();
  EmitExprCopy(op);
}

void BasicOptimizer::Emit(UnconditionalBranch& op) {
  if (!ResolveBranchIndirection(op)) {
    FlushCurrentLocals();
    EmitCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitBranchCompare(BranchCompare<T>& op) {
  TryReplaceVar(op.left_idx);
  TryReplaceVar(op.right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<T>>(op.right_idx);
    if (right_imm) {
      auto repl = BranchCompareImm<T>(op.dest, op.left_idx, op.op, right_imm->value);
      Emit(repl);
      return;
    }
  }

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      CmpType flipped = op.op;

      switch (op.op) {
        case CmpType::Eq:
        case CmpType::Ne: break;
        case CmpType::Lt: flipped = CmpType::Gt; break;
        case CmpType::Le: flipped = CmpType::Ge; break;
        case CmpType::Gt: flipped = CmpType::Lt; break;
        case CmpType::Ge: flipped = CmpType::Le; break;
      }
      auto repl = BranchCompareImm<T>(op.dest, op.right_idx, flipped, left_imm->value);
      Emit(repl);
      return;
    }
  }

  if (!ResolveBranchIndirection(op)) {
    FlushCurrentLocals();
    EmitCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitBranchCompareImm(BranchCompareImm<T>& op) {
  TryReplaceVar(op.left_idx);

  if constexpr(!std::is_same_v<T, Pointer>) {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      CmpType flipped = op.op;

      auto emit_unconditional = [&] {
        auto repl = UnconditionalBranch(op.dest);
        Emit(repl);
      };

      switch (op.op) {
        case CmpType::Eq: if (left_imm->value == op.right) emit_unconditional(); break;
        case CmpType::Ne: if (left_imm->value != op.right) emit_unconditional(); break;
        case CmpType::Lt: if (left_imm->value <  op.right) emit_unconditional(); break;
        case CmpType::Le: if (left_imm->value <= op.right) emit_unconditional(); break;
        case CmpType::Gt: if (left_imm->value >  op.right) emit_unconditional(); break;
        case CmpType::Ge: if (left_imm->value >= op.right) emit_unconditional(); break;
      }
      return;
    }
  }

  if (!ResolveBranchIndirection(op)) {
    FlushCurrentLocals();
    EmitCopy(op);
  }
}

void BasicOptimizer::Emit(Select& op) {
  TryReplaceVar(op.idx);
  FlushCurrentLocals();

  auto* val_imm = TryGetVarDirective<Imm<i64>>(op.idx);
  if (val_imm) {
    UnconditionalBranch repl{0};
    if (op.table.contains(val_imm->value)) {
      repl = UnconditionalBranch(op.table.at(val_imm->value));
    }
    else if (op._default) {
      repl = UnconditionalBranch(op._default);
    }
    else {
      throw std::runtime_error("Invalid switch selection with constant expression");
    }
    Emit(repl);
    return;
  }

  EmitCopy(op);
}

template<typename T>
void BasicOptimizer::EmitAddToPointer(AddToPointer<T>& op) {
  TryReplaceVar(op.ptr_idx);
  TryReplaceVar(op.right_idx);

  auto* right_imm = TryGetVarDirective<Imm<T>>(op.right_idx);
  if (right_imm) {
    if (right_imm->value == 0) {
      var_replacement[op.idx] = op.ptr_idx;
    }
    else if (op.op == PtrAddType::Add) {
      auto repl = AddToPointerImm(op.idx, op.ptr_idx, op.stride, right_imm->value);
      Emit(repl);
    }
    else {
      auto repl = AddToPointerImm(op.idx, op.ptr_idx, op.stride, -right_imm->value);
      Emit(repl);
    }
    return;
  }

  // todo: is this correct behavior?
  if (var_aliases.contains(op.ptr_idx)) {
    var_aliases[op.idx] = var_aliases.at(op.ptr_idx);
  }

  EmitExprCopy(op);
}

void BasicOptimizer::Emit(AddToPointerImm& op) {
  TryReplaceVar(op.ptr_idx);
  if (op.right == 0) {
    var_replacement[op.idx] = op.ptr_idx;
    return;
  }

  if (var_aliases.contains(op.ptr_idx) && op.right == 0) {
    var_aliases[op.idx] = var_aliases.at(op.ptr_idx);
  }
  EmitExprCopy(op);
}

#define BACKEND_NAME BasicOptimizer
#include "calyx/backend/Templates.inl"

}