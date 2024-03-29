#include "BasicOptimizer.h"
#include "RemoveUnused.h"
#include "IRCompare.h"
#include "Cast.h"
#include "Is.h"
#include "Containers.h"
#include "CustomAssert.h"
#include "Algorithm.h"


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
      auto candidate_block = deps.var_graph.at(var_idx).pos_made.first;
      auto ancestor = deps.CommonBlockAncestor(candidate_block, current_new_block_idx);

      // todo: shift directives back for earlier ancestor blocks
      if (ancestor == current_new_block_idx || ancestor == candidate_block) {
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

      // update block graph
      // blocks reached from indirect branch are now reached from the current_new_block_idx
      auto& link_from = deps.block_graph.at(link->dest).from;
      link_from.erase(op.dest);
      link_from.insert(current_new_block_idx);

      op.dest = link->dest;
      op.Emit(*this);
      return true;
    }
  }
  return false;
}

template<typename T>
const T* BasicOptimizer::TryGetVarDirective(var_index_t idx) const {
  auto [block, in_block] = vars_found.at(idx);
  auto& directive = new_program.blocks.at(block)[in_block];
  if (IsType<T>(directive)) {
    return cotyl::unique_ptr_cast<T>(directive);
  }
  return nullptr;
}

void BasicOptimizer::LinkBlock(block_label_t next_block) {
  // we need the current block to have no outputs
  cotyl::Assert(deps.block_graph.at(current_new_block_idx).to.empty());
  // sanity checks that the current block is indeed the only input for the linked block
  cotyl::Assert(deps.block_graph.at(next_block).from.size() == 1);
  // the ID should already have been updated from the old_block_idx to the new_block_idx earlier
  // if they differ at all
  cotyl::Assert(*deps.block_graph.at(next_block).from.begin() == current_new_block_idx);

  // change block inputs for jumped blocks
  for (auto block_idx : deps.block_graph.at(next_block).to) {
    deps.block_graph.at(block_idx).from.erase(next_block);
    deps.block_graph.at(block_idx).from.insert(current_new_block_idx);

    // update current block output to determine new upward closure to remove unused blocks right away
    deps.block_graph.at(current_new_block_idx).to.insert(block_idx);
  }

  // erase linked block
  deps.block_graph.erase(next_block);

  // update the block graph
  const auto closure = deps.UpwardClosure({visited.begin(), visited.end()});
  const auto not_in_closure = [&](const block_label_t& block_idx) { return !closure.contains(block_idx); };

  for (auto& [block_idx, deps] : deps.block_graph) {
    cotyl::erase_if(deps.to, not_in_closure);
    cotyl::erase_if(deps.from, not_in_closure);
  }

  // make current new_block_idx have empty output again
  deps.block_graph.at(current_new_block_idx).to = {};
  current_old_block_idx = next_block;
}

void BasicOptimizer::EmitProgram(const Program& _program) {
  // find initial dependencies
  new_program.functions    = std::move(_program.functions);
  new_program.globals      = std::move(_program.globals);
  new_program.strings      = std::move(_program.strings);

  for (const auto& [symbol, entry] : new_program.functions) {
    visited = {};
    todo = {entry};
    vars_found = {};

    while (!todo.empty()) {
      cotyl::Assert(locals.empty());

      current_old_block_idx = current_new_block_idx = *todo.begin();
      deps.pos.first = current_new_block_idx;  // set this properly to determine the new graph right
      // clear block dependencies
      deps.block_graph[current_new_block_idx] = {};
      reachable = true;
      todo.erase(todo.begin());

      auto inserted = new_program.blocks.emplace(current_new_block_idx, calyx::Program::block_t{}).first;
      current_block = &inserted->second;

      deps.pos.second = 0;
      while (!visited.contains(current_old_block_idx)) {
        const auto block = current_old_block_idx;
        visited.insert(block);
        for (const auto& directive : _program.blocks.at(block)) {
          deps.pos.second++;
          directive->Emit(*this);
          if (!reachable) break;  // hit return statement/branch
          if (current_old_block_idx != block) break;  // jumped to linked block
        }
      }
    }
  }
  RemoveUnused(new_program);
}

void BasicOptimizer::Emit(const AllocateLocal& op) {
  EmitCopy(op);
}

void BasicOptimizer::Emit(const DeallocateLocal& op) {
  EmitCopy(op);
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

template<typename To, typename From>
void BasicOptimizer::EmitCast(const Cast<To, From>& _op) {
  auto op = CopyDirective(_op);
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
          EmitRepl<Imm<calyx_upcast_t<To>>>(op.idx, (To)right_directive->value.value);
          return;
        }
      }
      else if constexpr(std::is_same_v<To, Pointer>) {
        // todo: pointer immediates
      }
      else{
        if (right_directive) {
          EmitRepl<Imm<calyx_upcast_t<To>>>(op.idx, (To)right_directive->value);
          return;
        }
      }
      EmitExprCopy(op);
    }
  }
}

template<typename T>
void BasicOptimizer::EmitLoadLocal(const LoadLocal<T>& op) {
  if (locals.contains(op.loc_idx)) {
    // local value stored in IR var
    auto& repl = locals.at(op.loc_idx).replacement;
    repl->idx = op.idx;
    repl->Emit(*this);
  }
  else {
    EmitExprCopy(op);
  }
}

void BasicOptimizer::Emit(const LoadLocalAddr& op) {
  var_aliases[op.idx] = op.loc_idx;
  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitStoreLocal(const StoreLocal<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.src);
  if constexpr(!std::is_same_v<T, Struct>) {
    var_index_t aliases = 0;
    if (var_aliases.contains(op.src)) {
      // var alias stored to local (i.e. int var; int* al = &var; int* al2 = al;)
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
void BasicOptimizer::EmitLoadGlobal(const LoadGlobal<T>& op) {
  EmitExprCopy(op);
}

void BasicOptimizer::Emit(const LoadGlobalAddr& op) {
  auto replaced = FindExprResultReplacement(op, [&](auto& op, auto& candidate) {
    return candidate.symbol == op.symbol;
  });
  if (!replaced) {
    EmitExprCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitStoreGlobal(const StoreGlobal<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.src);
  EmitCopy(op);
}

template<typename T>
void BasicOptimizer::EmitLoadFromPointer(const LoadFromPointer<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.ptr_idx);
  if (var_aliases.contains(op.ptr_idx)) {
    // pointer aliases local variable
    auto alias = var_aliases.at(op.ptr_idx);
    if (locals.contains(alias) && op.offset == 0) {
      // IR var holds local value, emit aliased local replacement
      auto& repl = locals.at(alias).replacement;
      repl->idx = op.idx;
      repl->Emit(*this);
    }
    else {
      // load aliased local directly
      EmitRepl<LoadLocal<T>>(op.idx, alias, op.offset);
    }
    return;
  }
  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitStoreToPointer(const StoreToPointer<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.src);
  TryReplaceVar(op.ptr_idx);
  if constexpr(!std::is_same_v<T, Struct>) {
    if (var_aliases.contains(op.ptr_idx) && op.offset == 0) {
      auto alias = var_aliases.at(op.ptr_idx);

      // storing aliased variable to another alias
      // i.e. int* zalias1 = &z; int** zaliasptr; *zaliasptr = zalias1;
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
void BasicOptimizer::EmitCall(const Call<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.fn_idx);
  for (auto& [var_idx, arg] : op.args) {
    TryReplaceVar(var_idx);
  }
  for (auto& [var_idx, arg] : op.var_args) {
    TryReplaceVar(var_idx);
  }

  auto* fn_adglb = TryGetVarDirective<LoadGlobalAddr>(op.fn_idx);
  if (fn_adglb) {
    EmitRepl<CallLabel<T>>(op.idx, fn_adglb->symbol, std::move(op.args), std::move(op.var_args));
    return;
  }

  vars_found[op.idx] = std::make_pair(current_new_block_idx, current_block->size());
  EmitCopy(op);
}

template<typename T>
void BasicOptimizer::EmitCallLabel(const CallLabel<T>& _op) {
  auto op = CopyDirective(_op);
  for (auto& [var_idx, arg] : op.args) {
    TryReplaceVar(var_idx);
  }
  for (auto& [var_idx, arg] : op.var_args) {
    TryReplaceVar(var_idx);
  }

  vars_found[op.idx] = std::make_pair(current_new_block_idx, current_block->size());
  EmitCopy(op);
}

void BasicOptimizer::Emit(const ArgMakeLocal& op) {
  EmitCopy(op);
}

template<typename T>
void BasicOptimizer::EmitReturn(const Return<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.idx);
  // no need to flush locals right before a return
  locals = {};
  EmitCopy(op);
  reachable = false;
}

template<typename T>
void BasicOptimizer::EmitImm(const Imm<T>& op) {
  auto replaced = FindExprResultReplacement(op, [&](auto& op, auto& candidate) {
    return candidate.value == op.value;
  });
  if (!replaced) {
    EmitExprCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitUnop(const Unop<T>& _op) {
  auto op = CopyDirective(_op);
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
      }
      EmitRepl<Imm<T>>(op.idx, result);
      return;
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitBinop(const Binop<T>& op_) {
  auto op = CopyDirective(op_);
  TryReplaceVar(op.left_idx);
  TryReplaceVar(op.right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<T>>(op.right_idx);
    if (right_imm) {
      EmitRepl<BinopImm<T>>(op.idx, op.left_idx, op.op, right_imm->value);
      return;
    }

    // fold unary expression into binop
    auto* right_unop = TryGetVarDirective<Unop<T>>(op.right_idx);
    if (right_unop) {
      if (right_unop->op == UnopType::Neg) {
        if (op.op == BinopType::Add) {
          EmitRepl<Binop<T>>(op.idx, op.left_idx, BinopType::Sub, right_unop->right_idx);
          return;
        }
        else if (op.op == BinopType::Sub) {
          EmitRepl<Binop<T>>(op.idx, op.left_idx, BinopType::Add, right_unop->right_idx);
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
          EmitRepl<BinopImm<T>>(op.idx, op.right_idx, op.op, left_imm->value);
          return;
        }
        case BinopType::Sub: {
          EmitRepl<BinopImm<T>>(op.idx, -left_imm->value, BinopType::Add, op.right_idx);
          return;
        }
        case BinopType::Div:
        case BinopType::Mod:
          // non-commutative
          break;
      }
    }

    // fold unop into binop
    auto* left_unop = TryGetVarDirective<Unop<T>>(op.left_idx);
    if (left_unop) {
      if (left_unop->op == UnopType::Neg) {
        if (op.op == BinopType::Add) {
          EmitRepl<Binop<T>>(op.idx, op.right_idx, BinopType::Sub, left_unop->right_idx);
          return;
        }
      }
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitBinopImm(const BinopImm<T>& op_) {
  auto op = CopyDirective(op_);
  TryReplaceVar(op.left_idx);

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
      EmitRepl<Imm<T>>(op.idx, result);
      return;
    }

    // result (v2 = (v1 @second b) @first a
    auto* left_binim = TryGetVarDirective<BinopImm<T>>(op.left_idx);
    if (left_binim) {
      using op_pair_t = std::pair<BinopType, BinopType>;
      using result_pair_t = std::pair<BinopType, T(*)(T, T)>;
      using enum BinopType;

      static const cotyl::unordered_map<op_pair_t, result_pair_t> binim_fold = {
              { {Add, Add}, {Add, [](T a, T b) { return a + b; }} },
              { {Add, Sub}, {Add, [](T a, T b) { return a - b; }} },
              { {Sub, Add}, {Add, [](T a, T b) { return b - a; }} },
              { {Sub, Sub}, {Sub, [](T a, T b) { return b + a; }} },
              { {Mul, Mul}, {Mul, [](T a, T b) { return a * b; }} },
              // note: (v1 / b) / a = v1 // (a * b) for integers too
              { {Div, Div}, {Div, [](T a, T b) { return a * b; }} },
              { {BinAnd, BinAnd}, {BinAnd, [](T a, T b) -> T { if constexpr(calyx::is_calyx_integral_type_v<T>) return a & b; return 0; }} },
              { {BinOr,  BinOr }, {BinOr,  [](T a, T b) -> T { if constexpr(calyx::is_calyx_integral_type_v<T>) return a | b; return 0; }} },
              { {BinXor, BinXor}, {BinXor, [](T a, T b) -> T { if constexpr(calyx::is_calyx_integral_type_v<T>) return a ^ b; return 0; }} },
      };

      if (binim_fold.contains({op.op, left_binim->op})) {
        const auto [repl_op, repl_fn] = binim_fold.at({op.op, left_binim->op});
        EmitRepl<BinopImm<T>>(op.idx, left_binim->left_idx, repl_op, repl_fn(op.right, left_binim->right));
        return;
      }
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitShift(const Shift<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.left_idx);
  TryReplaceVar(op.right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<u32>>(op.right_idx);
    if (right_imm) {
      EmitRepl<ShiftImm<T>>(op.idx, op.left_idx, op.op, right_imm->value);
      return;
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitShiftImm(const ShiftImm<T>& _op) {
  auto op = CopyDirective(_op);
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

      EmitRepl<Imm<T>>(op.idx, result);
      return;
    }
  }
  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitCompare(const Compare<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.left_idx);
  TryReplaceVar(op.right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<T>>(op.right_idx);
    if (right_imm) {
      EmitRepl<CompareImm<T>>(op.idx, op.left_idx, op.op, right_imm->value);
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
      EmitRepl<CompareImm<T>>(op.idx, op.right_idx, flipped, left_imm->value);
      return;
    }
  }

  EmitExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitCompareImm(const CompareImm<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.left_idx);

  if constexpr(!std::is_same_v<T, Pointer>) {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      i32 result = 0;

      switch (op.op) {
        case CmpType::Eq: result = left_imm->value == op.right; break;
        case CmpType::Ne: result = left_imm->value != op.right; break;
        case CmpType::Lt: result = left_imm->value <  op.right; break;
        case CmpType::Le: result = left_imm->value <= op.right; break;
        case CmpType::Gt: result = left_imm->value >  op.right; break;
        case CmpType::Ge: result = left_imm->value >= op.right; break;
      }
      EmitRepl<Imm<i32>>(op.idx, result);
      return;
    }
  }

  EmitExprCopy(op);
}

void BasicOptimizer::Emit(const UnconditionalBranch& _op) {
  auto op = CopyDirective(_op);
  if (!ResolveBranchIndirection(op)) {
    const auto& block_deps = this->deps.block_graph.at(current_new_block_idx);
    if (block_deps.to.empty()) {
      // no dependencies so far, we can link this block if the next block only has one input
      if (!visited.contains(op.dest) && this->deps.block_graph.at(op.dest).from.size() == 1) {
        LinkBlock(op.dest);
        return;
      }
    }

    // only flush locals on the unconditional branch non-linked blocks
    FlushCurrentLocals();
    if (!visited.contains(op.dest)) {
      todo.insert(op.dest);
    }
    EmitCopy(op);
    reachable = false;
  }
}

template<typename T>
void BasicOptimizer::EmitBranchCompare(const BranchCompare<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.left_idx);
  TryReplaceVar(op.right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<T>>(op.right_idx);
    if (right_imm) {
      EmitRepl<BranchCompareImm<T>>(op.dest, op.left_idx, op.op, right_imm->value);
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
      EmitRepl<BranchCompareImm<T>>(op.dest, op.right_idx, flipped, left_imm->value);
      return;
    }
  }

  if (!ResolveBranchIndirection(op)) {
    FlushCurrentLocals();
    if (!visited.contains(op.dest)) {
      todo.insert(op.dest);
    }
    EmitCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitBranchCompareImm(const BranchCompareImm<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.left_idx);

  if constexpr(!std::is_same_v<T, Pointer>) {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      CmpType flipped = op.op;

      auto emit_unconditional = [&] {
        EmitRepl<UnconditionalBranch>(op.dest);
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
    if (!visited.contains(op.dest)) {
      todo.insert(op.dest);
    }
    EmitCopy(op);
  }
}

void BasicOptimizer::Emit(const Select& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.idx);
  FlushCurrentLocals();

  auto* val_imm = TryGetVarDirective<Imm<i64>>(op.idx);
  if (val_imm) {
    if (op.table.contains(val_imm->value)) {
      EmitRepl<UnconditionalBranch>(op.table.at(val_imm->value));
      return;
    }
    else if (op._default) {
      EmitRepl<UnconditionalBranch>(op._default);
      return;
    }
    else {
      throw std::runtime_error("Invalid switch selection with constant expression");
    }
  }

  EmitCopy(op);
  reachable = false;
}

template<typename T>
void BasicOptimizer::EmitAddToPointer(const AddToPointer<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.ptr_idx);
  TryReplaceVar(op.right_idx);

  auto* right_imm = TryGetVarDirective<Imm<T>>(op.right_idx);
  if (right_imm) {
    if (right_imm->value == 0) {
      var_replacement[op.idx] = op.ptr_idx;
    }
    else if (op.op == PtrAddType::Add) {
      EmitRepl<AddToPointerImm>(op.idx, op.ptr_idx, op.stride, right_imm->value);
    }
    else {
      EmitRepl<AddToPointerImm>(op.idx, op.ptr_idx, op.stride, -right_imm->value);
    }
    return;
  }

  // todo: is this correct behavior?
//  if (var_aliases.contains(op.ptr_idx)) {
//    var_aliases[op.idx] = var_aliases.at(op.ptr_idx);
//  }

  EmitExprCopy(op);
}

void BasicOptimizer::Emit(const AddToPointerImm& _op) {
  auto op = CopyDirective(_op);
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
#pragma clang diagnostic pop

}