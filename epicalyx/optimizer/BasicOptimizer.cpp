#include "BasicOptimizer.h"
#include "RemoveUnused.h"
#include "IRCompare.h"
#include "Cast.h"
#include "Is.h"
#include "Containers.h"
#include "CustomAssert.h"
#include "Algorithm.h"


namespace epi {

// uses that block labels are ordered topologically
block_label_t BasicOptimizer::CommonBlockAncestor(block_label_t first, block_label_t second) const {
  cotyl::set<block_label_t> ancestors{first, second};

  // we use the fact that in general block1 > block2 then block1 can never be an ancestor of block2
  // it may happen for loops, but then the loop entry is the minimum block, so we want to go there
  cotyl::unordered_set<block_label_t> ancestors_considered{};

  while (ancestors.size() > 1) {
    auto max_ancestor = *ancestors.rbegin();
    ancestors.erase(std::prev(ancestors.end()));
    ancestors_considered.emplace(max_ancestor);
    if (!old_deps.block_graph.Has(max_ancestor)) [[unlikely]] {
      return 0;
    }

    auto& block = old_deps.block_graph.At(max_ancestor);
    if (block.from.empty()) [[unlikely]] {
      return 0;
    }
    for (auto dep : block.from) {
      if (dep < max_ancestor || !ancestors_considered.contains(dep)) {
        ancestors.insert(dep);
      }
    }
  }

  // at this point only one ancestor should be left
  return *ancestors.begin();
}

template<class BadPred, class GoodPred>
bool BasicOptimizer::NoBadBeforeGoodAllPaths(BadPred bad, GoodPred good, program_pos_t pos) const {
  cotyl::unordered_set<block_label_t> todo{};
  cotyl::unordered_set<block_label_t> done{};

  const auto register_branch = [&](const block_label_t& block_idx) {
    if (!done.contains(block_idx)) {
      todo.emplace(block_idx);
    }
  };

  while (true) {
    bool _reachable = true;

    // use new program block if already made
    // EXCEPT for the current block, as it is still being built,
    // meaning branches may be missing
    const auto& block = program.blocks.at(pos.first);

    while (_reachable && pos.second < block.size()) {
      const auto& op = block[pos.second];
      switch (op->cls) {
        case Directive::Class::UnconditionalBranch:
          _reachable = false;
        case Directive::Class::ConditionalBranch: 
          register_branch(cotyl::unique_ptr_cast<Branch>(op)->dest);
          break;
        case Directive::Class::Select: {
          auto* select = cotyl::unique_ptr_cast<Select>(op);
          for (auto [val, block] : select->table) {
            register_branch(block);
          }
          if (select->_default) {
            register_branch(select->_default);
          }
          _reachable = false;
          break;
        }
        case Directive::Class::Return:
          _reachable = false;
          break;
        default:
          break;
      }

      if (bad(op, pos)) return false;
      if (good(op, pos)) break;
      pos.second++;
    }

    if (todo.empty()) break;

    pos.first = *todo.begin();
    pos.second = 0;

    // add to done here, and not before, since we may 
    // have started halfway through the first block
    done.emplace(pos.first);
    todo.erase(todo.begin());
  }
  
  return true;
}

bool BasicOptimizer::ShouldFlushLocal(var_index_t loc_idx, const Local& local) {
  const auto& old_local = old_deps.local_graph.at(loc_idx);
  cotyl::unordered_set<program_pos_t> reads  = {old_local.reads.begin(), old_local.reads.end()};
  cotyl::unordered_set<program_pos_t> writes = {old_local.writes.begin(), old_local.writes.end()};
  
  static const cotyl::unordered_set<size_t> alias_blocking_tids = {
    Call<i32>::GetTID(),
    Call<u32>::GetTID(),
    Call<i64>::GetTID(),
    Call<u64>::GetTID(),
    Call<float>::GetTID(),
    Call<double>::GetTID(),
    Call<Pointer>::GetTID(),
    Call<Struct>::GetTID(),
    Call<void>::GetTID(),
    CallLabel<i32>::GetTID(),
    CallLabel<u32>::GetTID(),
    CallLabel<i64>::GetTID(),
    CallLabel<u64>::GetTID(),
    CallLabel<float>::GetTID(),
    CallLabel<double>::GetTID(),
    CallLabel<Pointer>::GetTID(),
    CallLabel<Struct>::GetTID(),
    CallLabel<void>::GetTID(),
    LoadFromPointer<i8>::GetTID(),
    LoadFromPointer<u8>::GetTID(),
    LoadFromPointer<i16>::GetTID(),
    LoadFromPointer<u16>::GetTID(),
    LoadFromPointer<i32>::GetTID(),
    LoadFromPointer<u32>::GetTID(),
    LoadFromPointer<i64>::GetTID(),
    LoadFromPointer<u64>::GetTID(),
    LoadFromPointer<float>::GetTID(),
    LoadFromPointer<double>::GetTID(),
    LoadFromPointer<Struct>::GetTID(),
    LoadFromPointer<Pointer>::GetTID(),
  };

  // local does not need to be flushed if no "bad" operation (load) happens
  // before a "good" operation (store)
  return !NoBadBeforeGoodAllPaths(
    [&](const pDirective& op, const program_pos_t& pos) -> bool {
      // we need to flush the local if it is read before it is stored again
      // or if any aliased variable exists, and one of the following happens:
      // - a pointer read (potentially reading the local's (aliased) value)
      // - a function call (potentially passing the local's address, potentially
      //                    reading the local's value)
      if (reads.contains(pos)) return true;
      if (!old_local.aliased_by.empty()) {
        if (alias_blocking_tids.contains(op->type_id)) {
          return true;
        }
      }

      // operation is OK, does not affect local
      return false;
    },
    [&](const pDirective& op, const program_pos_t& pos) -> bool {
      // write: stop searching path
      return writes.contains(pos);
    },
    current_old_pos
  );
}

void BasicOptimizer::FlushLocal(var_index_t loc_idx, Local&& local) {
  if (local_writes.contains(loc_idx)) {
    if (ShouldFlushLocal(loc_idx, local)) Output(std::move(local.store));
    local_writes.erase(loc_idx);
  }
}

void BasicOptimizer::FlushCurrentLocalWrites() {
  for (auto& [loc_idx, local] : local_writes) {
    if (ShouldFlushLocal(loc_idx, local)) {
      Output(std::move(local.store));
    }
  }
  local_writes.clear();
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
      auto candidate_block = loc.first;
      // todo: make generic, call on new_block_graph
      auto ancestor = CommonBlockAncestor(candidate_block, current_new_block_idx);

      // todo: shift directives back for earlier ancestor blocks
      // todo: improve this
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


void BasicOptimizer::ResolveBranchIndirection(block_label_t& dest) {
  cotyl::unordered_set<block_label_t> found{dest};

  while (program.blocks.at(dest).size() == 1) {
    // single branch block
    // recursion resolves single branch chains
    auto& link_directive = program.blocks.at(dest)[0];
    if (!IsType<UnconditionalBranch>(link_directive)) {
      break;
    }
    auto* link = cotyl::unique_ptr_cast<UnconditionalBranch>(link_directive);

    dest = link->dest;
    
    // tight infinite loop of single branch blocks
    if (found.contains(dest)) break;
    found.emplace(dest);
  }
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
  cotyl::Assert(new_block_graph.At(current_new_block_idx).to.empty());
  // sanity checks that the current block is indeed the only input for the linked block
  cotyl::Assert(old_deps.block_graph.At(next_block).from.size() == 1);
  // the ID should already have been updated from the old_block_idx to the new_block_idx earlier
  // if they differ at all
  cotyl::Assert(*old_deps.block_graph.At(next_block).from.begin() == current_old_pos.first);

  block_links.emplace(next_block, program_pos_t{current_new_block_idx, current_block->size()});
  current_old_pos.first = next_block;
}

void BasicOptimizer::RemoveUnreachableBlockEdgesRecurse(block_label_t block) {
  auto& node = old_deps.block_graph[block];

  while (!node.to.empty()) {
    const auto to_idx = *node.to.begin();
    node.to.erase(node.to.begin());

    const auto& to_from = old_deps.block_graph.At(to_idx).from;
    if (std::count_if(to_from.begin(), to_from.end(), [to_idx](const auto idx) { return idx != to_idx; }) == 0) {
      RemoveUnreachableBlockEdgesRecurse(to_idx);
    }
  }
}

void BasicOptimizer::RemoveUnreachableBlockEdges(block_label_t block) {
  // we want to remove edges from the old block graph if they are unreachable
  // this improves block linking
  // we have to make sure NOT to remove any edges that are already added to the 
  // current new block, as the block that was passed was partially linked,
  // and conditional branches may have happened in the reachable part, as well
  // as (usually) some unconditional branch
  const auto& current_node = new_block_graph.At(current_new_block_idx);
  const auto& node = old_deps.block_graph.At(block);
  cotyl::flat_set<block_label_t> to_remove{};
  for (const auto to_idx : node.to) {
    if (!current_node.to.contains(to_idx)) {
      to_remove.emplace(to_idx);
    }
  }

  for (const auto to_idx : to_remove) {
    old_deps.block_graph.RemoveEdge(block, to_idx);
    const auto& to_from = old_deps.block_graph.At(to_idx).from;

    // don't care for loops when removing unused blocks
    if (std::none_of(to_from.begin(), to_from.end(), [to_idx](const auto idx) { return idx != to_idx; })) {
      RemoveUnreachableBlockEdgesRecurse(to_idx);
    }
  }
}

void BasicOptimizer::EmitProgram(const Program& _program) {
  // find initial dependencies
  new_program.functions    = std::move(_program.functions);
  new_program.globals      = std::move(_program.globals);
  new_program.strings      = std::move(_program.strings);

  for (const auto& [symbol, entry] : new_program.functions) {
    new_block_graph = {};
    todo = {entry};
    vars_found.clear();

    while (!todo.empty()) {
      cotyl::Assert(local_writes.empty());

      current_old_pos.first = current_new_block_idx = *todo.begin();
      reachable = true;
      todo.erase(todo.begin());

      auto inserted = new_program.blocks.emplace(current_new_block_idx, calyx::Program::block_t{}).first;
      current_block = &inserted->second;
      auto& node = new_block_graph.EmplaceNodeIfNotExists(current_new_block_idx, nullptr);
      if (node.value) continue;  // we have already emitted this block
      node.value = current_block;

      bool block_finished;
      do {
        block_finished = true;
        block_links.emplace(current_old_pos.first, program_pos_t{current_new_block_idx, current_block->size()});
        const auto block = current_old_pos.first;
        current_old_pos.second = 0;
        for (const auto& directive : _program.blocks.at(block)) {
          directive->Emit(*this);
          // hit return statement/branch, block finished
          if (!reachable) { RemoveUnreachableBlockEdges(block); break; }
          // jumped to linked block, block NOT finished
          if (current_old_pos.first != block) { block_finished = false; break; }
          current_old_pos.second++;  
        }
      } while (!block_finished);
    }
  }
  while (RemoveUnused(new_program));
}

void BasicOptimizer::Emit(const AllocateLocal& op) {
  OutputCopy(op);
}

void BasicOptimizer::Emit(const DeallocateLocal& op) {
  OutputCopy(op);
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

template<typename To, typename From>
void BasicOptimizer::EmitCast(const Cast<To, From>& _op) {
  auto op = CopyDirective(_op);
  if constexpr(std::is_same_v<To, From>) {
    var_replacement[op->idx] = op->right_idx;
    return;
  }
  else {
    TryReplaceVar(op->right_idx);
    auto replaced = FindExprResultReplacement(*op, [](auto& op, auto& candidate) {
      return candidate.right_idx == op.right_idx;
    });
    if (replaced) {
      return;
    }

    auto* right_directive = TryGetVarDirective<Imm<From>>(op->right_idx);
    if constexpr(std::is_same_v<From, Pointer>) {
      if (right_directive) {
        EmitRepl<Imm<calyx_upcast_t<To>>>(op->idx, (To)right_directive->value.value);
        return;
      }
    }
    else if constexpr(std::is_same_v<To, Pointer>) {
      // todo: pointer immediates
    }
    else{
      if (right_directive) {
        EmitRepl<Imm<calyx_upcast_t<To>>>(op->idx, (To)right_directive->value);
        return;
      }
    }
    OutputExpr(std::move(op));
  }
}

template<typename T>
void BasicOptimizer::EmitLoadLocal(const LoadLocal<T>& op) {
  if (local_writes.contains(op.loc_idx)) {
    // local value stored in IR var
    auto& repl = local_writes.at(op.loc_idx).replacement;
    repl->idx = op.idx;
    repl->Emit(*this);
  }
  else {
    OutputExprCopy(op);
  }
}

void BasicOptimizer::Emit(const LoadLocalAddr& op) {
  OutputExprCopy(op);
}

template<typename T>
void BasicOptimizer::EmitStoreLocal(const StoreLocal<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->src);
  if constexpr(!std::is_same_v<T, Struct>) {
    var_index_t aliases = 0;
    if (old_deps.var_graph.at(op->src).aliases) {
      // var alias stored to local (i.e. int var; int* al = &var; int* al2 = al;)
      aliases = old_deps.var_graph.at(op->src).aliases;
    }

    const auto loc_idx = op->loc_idx;  // op will be moved when assigning
    local_writes[loc_idx] = Local{
            .aliases = aliases,
            .replacement = std::make_unique<Cast<T, calyx_upcast_t<T>>>(0, op->src),
            .store = std::move(op)
    };
  }
  else {
    Output(std::move(op));
  }
}

template<typename T>
void BasicOptimizer::EmitLoadGlobal(const LoadGlobal<T>& op) {
  OutputExprCopy(op);
}

void BasicOptimizer::Emit(const LoadGlobalAddr& op) {
  auto replaced = FindExprResultReplacement(op, [](auto& op, auto& candidate) {
    return candidate.symbol == op.symbol;
  });
  if (!replaced) {
    OutputExprCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitStoreGlobal(const StoreGlobal<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->src);
  Output(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitLoadFromPointer(const LoadFromPointer<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->ptr_idx);
  if (old_deps.var_graph.at(op->ptr_idx).aliases) {
    // pointer aliases local variable
    auto alias = old_deps.var_graph.at(op->ptr_idx).aliases;
    if (local_writes.contains(alias) && op->offset == 0) {
      // IR var holds local value, emit aliased local replacement
      auto& repl = local_writes.at(alias).replacement;
      repl->idx = op->idx;
      repl->Emit(*this);
    }
    else {
      // load aliased local directly
      EmitRepl<LoadLocal<T>>(op->idx, alias, op->offset);
    }
    return;
  }
  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitStoreToPointer(const StoreToPointer<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->src);
  TryReplaceVar(op->ptr_idx);
  if constexpr(!std::is_same_v<T, Struct>) {
    if (old_deps.var_graph.at(op->ptr_idx).aliases && op->offset == 0) {
      auto alias = old_deps.var_graph.at(op->ptr_idx).aliases;

      // storing aliased variable to another alias
      // i.e. int* zalias1 = &z; int** zaliasptr; *zaliasptr = zalias1;
      var_index_t aliases = 0;
      if (old_deps.var_graph.at(op->src).aliases) {
        aliases = old_deps.var_graph.at(op->src).aliases;
      }

      // replace alias
      local_writes[alias] = Local{
              .aliases = aliases,
              .replacement = std::make_unique<Cast<T, calyx_upcast_t<T>>>(0, op->src),
              .store = std::make_unique<StoreLocal<T>>(alias, op->src)
      };
      return;
    }
  }

  // need to flush local writes on pointer write
  FlushCurrentLocalWrites();
  Output(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitCall(const Call<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->fn_idx);
  for (auto& [var_idx, arg] : op->args) {
    TryReplaceVar(var_idx);
  }
  for (auto& [var_idx, arg] : op->var_args) {
    TryReplaceVar(var_idx);
  }

  auto* fn_adglb = TryGetVarDirective<LoadGlobalAddr>(op->fn_idx);
  if (fn_adglb) {
    EmitRepl<CallLabel<T>>(op->idx, fn_adglb->symbol, std::move(op->args), std::move(op->var_args));
    return;
  }

  vars_found[op->idx] = std::make_pair(current_new_block_idx, current_block->size());
  // need to flush local_writes on call, in case a pointer read/store happens
  FlushCurrentLocalWrites();
  Output(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitCallLabel(const CallLabel<T>& _op) {
  auto op = CopyDirective(_op);
  for (auto& [var_idx, arg] : op->args) {
    TryReplaceVar(var_idx);
  }
  for (auto& [var_idx, arg] : op->var_args) {
    TryReplaceVar(var_idx);
  }

  vars_found[op->idx] = std::make_pair(current_new_block_idx, current_block->size());
  // need to flush local_writes on call, in case a pointer read/store happens
  FlushCurrentLocalWrites();
  Output(std::move(op));
}

void BasicOptimizer::Emit(const ArgMakeLocal& op) {
  OutputCopy(op);
}

template<typename T>
void BasicOptimizer::EmitReturn(const Return<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->idx);
  // no need to flush local_writes right before a return
  local_writes.clear();
  Output(std::move(op));
  reachable = false;
}

template<typename T>
void BasicOptimizer::EmitImm(const Imm<T>& op) {
  auto replaced = FindExprResultReplacement(op, [](auto& op, auto& candidate) {
    return candidate.value == op.value;
  });
  if (!replaced) {
    OutputExprCopy(op);
  }
}

template<typename T>
void BasicOptimizer::EmitUnop(const Unop<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->right_idx);

  {
    auto* right_unop = TryGetVarDirective<Unop<T>>(op->right_idx);
    if (right_unop) {
      if (right_unop->op == op->op) {
        // ~ and - both cancel themselves out
        var_replacement[op->idx] = right_unop->right_idx;
        return;
      }
    }
    auto* right_imm = TryGetVarDirective<Imm<T>>(op->right_idx);
    if (right_imm) {
      T result;
      switch (op->op) {
        case UnopType::Neg: result = -right_imm->value; break;
        case UnopType::BinNot: if constexpr(calyx::is_calyx_integral_type_v<T>) result = ~right_imm->value; break;
      }
      EmitRepl<Imm<T>>(op->idx, result);
      return;
    }
  }

  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitBinop(const Binop<T>& op_) {
  auto op = CopyDirective(op_);
  TryReplaceVar(op->left_idx);
  TryReplaceVar(op->right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<T>>(op->right_idx);
    if (right_imm) {
      EmitRepl<BinopImm<T>>(op->idx, op->left_idx, op->op, right_imm->value);
      return;
    }

    // fold unary expression into binop
    auto* right_unop = TryGetVarDirective<Unop<T>>(op->right_idx);
    if (right_unop) {
      if (right_unop->op == UnopType::Neg) {
        if (op->op == BinopType::Add) {
          EmitRepl<Binop<T>>(op->idx, op->left_idx, BinopType::Sub, right_unop->right_idx);
          return;
        }
        else if (op->op == BinopType::Sub) {
          EmitRepl<Binop<T>>(op->idx, op->left_idx, BinopType::Add, right_unop->right_idx);
          return;
        }
      }
    }
  }

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op->left_idx);
    if (left_imm) {
      switch (op->op) {
        case BinopType::Add:
        case BinopType::BinAnd:
        case BinopType::BinOr:
        case BinopType::BinXor:
        case BinopType::Mul: {
          EmitRepl<BinopImm<T>>(op->idx, op->right_idx, op->op, left_imm->value);
          return;
        }
        case BinopType::Sub: {
          EmitRepl<BinopImm<T>>(op->idx, -left_imm->value, BinopType::Add, op->right_idx);
          return;
        }
        case BinopType::Div:
        case BinopType::Mod:
          // non-commutative
          break;
      }
    }

    // fold unop into binop
    auto* left_unop = TryGetVarDirective<Unop<T>>(op->left_idx);
    if (left_unop) {
      if (left_unop->op == UnopType::Neg) {
        if (op->op == BinopType::Add) {
          EmitRepl<Binop<T>>(op->idx, op->right_idx, BinopType::Sub, left_unop->right_idx);
          return;
        }
      }
    }
  }

  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitBinopImm(const BinopImm<T>& op_) {
  auto op = CopyDirective(op_);
  TryReplaceVar(op->left_idx);

  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op->left_idx);
    if (left_imm) {
      T result;
      switch (op->op) {
        case BinopType::Add: result = left_imm->value + op->right; break;
        case BinopType::Sub: result = left_imm->value - op->right; break;
        case BinopType::Mul: result = left_imm->value * op->right; break;
        case BinopType::Div: result = left_imm->value / op->right; break;
        default:
          if constexpr(is_calyx_integral_type_v<T>) {
            switch (op->op) {
              case BinopType::Mod:    result = left_imm->value % op->right; break;
              case BinopType::BinAnd: result = left_imm->value & op->right; break;
              case BinopType::BinOr:  result = left_imm->value | op->right; break;
              case BinopType::BinXor: result = left_imm->value ^ op->right; break;
              default: break;
            }
          }
          break;
      }
      EmitRepl<Imm<T>>(op->idx, result);
      return;
    }

    // result (v2 = (v1 @second b) @first a
    auto* left_binim = TryGetVarDirective<BinopImm<T>>(op->left_idx);
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

      if (binim_fold.contains({op->op, left_binim->op})) {
        const auto [repl_op, repl_fn] = binim_fold.at({op->op, left_binim->op});
        EmitRepl<BinopImm<T>>(op->idx, left_binim->left_idx, repl_op, repl_fn(op->right, left_binim->right));
        return;
      }
    }
  }

  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitShift(const Shift<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->left_idx);
  TryReplaceVar(op->right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<u32>>(op->right_idx);
    if (right_imm) {
      EmitRepl<ShiftImm<T>>(op->idx, op->left_idx, op->op, right_imm->value);
      return;
    }
  }

  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitShiftImm(const ShiftImm<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->left_idx);

  if (op->right == 0) {
    var_replacement[op->idx] = op->left_idx;
    return;
  }

  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op->left_idx);
    if (left_imm) {
      T result;
      switch (op->op) {
        case ShiftType::Left:  result = left_imm->value << op->right; break;
        case ShiftType::Right: result = left_imm->value >> op->right; break;
      }

      EmitRepl<Imm<T>>(op->idx, result);
      return;
    }
  }
  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitCompare(const Compare<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->left_idx);
  TryReplaceVar(op->right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<T>>(op->right_idx);
    if (right_imm) {
      EmitRepl<CompareImm<T>>(op->idx, op->left_idx, op->op, right_imm->value);
      return;
    }
  }

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op->left_idx);
    if (left_imm) {
      CmpType flipped = op->op;

      switch (op->op) {
        case CmpType::Eq:
        case CmpType::Ne: break;
        case CmpType::Lt: flipped = CmpType::Gt; break;
        case CmpType::Le: flipped = CmpType::Ge; break;
        case CmpType::Gt: flipped = CmpType::Lt; break;
        case CmpType::Ge: flipped = CmpType::Le; break;
      }
      EmitRepl<CompareImm<T>>(op->idx, op->right_idx, flipped, left_imm->value);
      return;
    }
  }

  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitCompareImm(const CompareImm<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->left_idx);

  if constexpr(!std::is_same_v<T, Pointer>) {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op->left_idx);
    if (left_imm) {
      i32 result = 0;

      switch (op->op) {
        case CmpType::Eq: result = left_imm->value == op->right; break;
        case CmpType::Ne: result = left_imm->value != op->right; break;
        case CmpType::Lt: result = left_imm->value <  op->right; break;
        case CmpType::Le: result = left_imm->value <= op->right; break;
        case CmpType::Gt: result = left_imm->value >  op->right; break;
        case CmpType::Ge: result = left_imm->value >= op->right; break;
      }
      EmitRepl<Imm<i32>>(op->idx, result);
      return;
    }

    // replace comparisons with added constants
    // i.e., change (left + 1) > 0 with left > -1
    auto* left_binim = TryGetVarDirective<BinopImm<T>>(op->left_idx);
    if (left_binim) {
      switch (left_binim->op) {
        case BinopType::Add:
          op->left_idx = left_binim->left_idx;
          op->right -= left_binim->right;
          break;
        case BinopType::Sub:
          op->left_idx = left_binim->left_idx;
          op->right += left_binim->right;
          break;
        case BinopType::BinXor: {
          if constexpr(std::is_integral_v<T>) {
            if (op->op == CmpType::Eq || op->op == CmpType::Ne) {
              op->left_idx = left_binim->left_idx;
              op->right ^= left_binim->right;
            }
          }
          break;
        }
        default:
          break;
      }
    }
  }

  OutputExpr(std::move(op));
}

void BasicOptimizer::Emit(const UnconditionalBranch& _op) {
  auto op = CopyDirective(_op);
  ResolveBranchIndirection(op->dest);
  const auto& block_deps = new_block_graph.At(current_new_block_idx);
  if (block_deps.to.empty()) {
    // no dependencies so far, we can link this block if the next block only has one input
    if (!new_block_graph.Has(op->dest) && old_deps.block_graph.At(op->dest).from.size() == 1) {
      LinkBlock(op->dest);
      return;
    }
  }

  // only flush local_writes on the unconditional branch non-linked blocks
  FlushCurrentLocalWrites();
  if (!new_block_graph.Has(op->dest)) {
    new_block_graph.AddNodeIfNotExists(op->dest, nullptr);
    new_block_graph.AddEdge(current_new_block_idx, op->dest);
    todo.insert(op->dest);
  }
  Output(std::move(op));
  reachable = false;
}

template<typename T>
void BasicOptimizer::EmitBranchCompare(const BranchCompare<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->left_idx);
  TryReplaceVar(op->right_idx);

  // replace right side
  {
    auto* right_imm = TryGetVarDirective<Imm<T>>(op->right_idx);
    if (right_imm) {
      EmitRepl<BranchCompareImm<T>>(op->dest, op->left_idx, op->op, right_imm->value);
      return;
    }
  }

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op->left_idx);
    if (left_imm) {
      CmpType flipped = op->op;

      // note: flipped, not converse, we are simply chainging
      // left < right with right > left for example
      switch (op->op) {
        case CmpType::Eq:
        case CmpType::Ne: break;
        case CmpType::Lt: flipped = CmpType::Gt; break;
        case CmpType::Le: flipped = CmpType::Ge; break;
        case CmpType::Gt: flipped = CmpType::Lt; break;
        case CmpType::Ge: flipped = CmpType::Le; break;
      }
      EmitRepl<BranchCompareImm<T>>(op->dest, op->right_idx, flipped, left_imm->value);
      return;
    }
  }

  ResolveBranchIndirection(op->dest);
  FlushCurrentLocalWrites();
  if (!new_block_graph.Has(op->dest)) {
    new_block_graph.AddNodeIfNotExists(op->dest, nullptr);
    new_block_graph.AddEdge(current_new_block_idx, op->dest);
    todo.insert(op->dest);
  }
  Output(std::move(op));
}

template<typename T>
void BasicOptimizer::EmitBranchCompareImm(const BranchCompareImm<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->left_idx);

  if constexpr(!std::is_same_v<T, Pointer>) {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op->left_idx);
    if (left_imm) {
      auto emit_unconditional = [&] {
        EmitRepl<UnconditionalBranch>(op->dest);
      };

      switch (op->op) {
        case CmpType::Eq: if (left_imm->value == op->right) emit_unconditional(); break;
        case CmpType::Ne: if (left_imm->value != op->right) emit_unconditional(); break;
        case CmpType::Lt: if (left_imm->value <  op->right) emit_unconditional(); break;
        case CmpType::Le: if (left_imm->value <= op->right) emit_unconditional(); break;
        case CmpType::Gt: if (left_imm->value >  op->right) emit_unconditional(); break;
        case CmpType::Ge: if (left_imm->value >= op->right) emit_unconditional(); break;
      }
      return;
    }

    // replace comparisons with added constants
    // i.e., change (left + 1) > 0 with left > -1
    auto* left_binim = TryGetVarDirective<BinopImm<T>>(op->left_idx);
    if (left_binim) {
      switch (left_binim->op) {
        case BinopType::Add:
          op->left_idx = left_binim->left_idx;
          op->right -= left_binim->right;
          break;
        case BinopType::Sub:
          op->left_idx = left_binim->left_idx;
          op->right += left_binim->right;
          break;
        case BinopType::BinXor: {
          if constexpr(std::is_integral_v<T>) {
            if (op->op == CmpType::Eq || op->op == CmpType::Ne) {
              op->left_idx = left_binim->left_idx;
              op->right ^= left_binim->right;
            }
          }
          break;
        }
        default:
          break;
      }
    }
  }

  ResolveBranchIndirection(op->dest);
  FlushCurrentLocalWrites();
  if (!new_block_graph.Has(op->dest)) {
    new_block_graph.AddNodeIfNotExists(op->dest, nullptr);
    new_block_graph.AddEdge(current_new_block_idx, op->dest);
    todo.insert(op->dest);
  }
  Output(std::move(op));
}

void BasicOptimizer::Emit(const Select& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->idx);

  auto* val_imm = TryGetVarDirective<Imm<i64>>(op->idx);
  if (val_imm) {
    if (op->table.contains(val_imm->value)) {
      EmitRepl<UnconditionalBranch>(op->table.at(val_imm->value));
      return;
    }
    else if (op->_default) {
      EmitRepl<UnconditionalBranch>(op->_default);
      return;
    }
    else {
      throw std::runtime_error("Invalid switch selection with constant expression");
    }
  }

  FlushCurrentLocalWrites();
  for (auto& [val, block_idx] : op->table) {
    ResolveBranchIndirection(block_idx);
    if (!new_block_graph.Has(block_idx)) {
      new_block_graph.AddNodeIfNotExists(block_idx, nullptr);
      new_block_graph.AddEdge(current_new_block_idx, block_idx);
      todo.insert(block_idx);
    }
  }
  if (op->_default) {
    ResolveBranchIndirection(op->_default);
    if (!new_block_graph.Has(op->_default)) {
      new_block_graph.AddNodeIfNotExists(op->_default, nullptr);
      new_block_graph.AddEdge(current_new_block_idx, op->_default);
      todo.insert(op->_default);
    }
  }

  Output(std::move(op));
  reachable = false;
}

template<typename T>
void BasicOptimizer::EmitAddToPointer(const AddToPointer<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->ptr_idx);
  TryReplaceVar(op->right_idx);

  auto* right_imm = TryGetVarDirective<Imm<T>>(op->right_idx);
  if (right_imm) {
    if (right_imm->value == 0) {
      var_replacement[op->idx] = op->ptr_idx;
    }
    else {
      EmitRepl<AddToPointerImm>(op->idx, op->ptr_idx, op->stride, right_imm->value);
    }
    return;
  }

  OutputExpr(std::move(op));
}

void BasicOptimizer::Emit(const AddToPointerImm& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op->ptr_idx);
  if (op->right == 0) {
    var_replacement[op->idx] = op->ptr_idx;
    return;
  }

  if (old_deps.var_graph[op->ptr_idx].aliases && op->right == 0) {
    old_deps.var_graph[op->idx].aliases = old_deps.var_graph[op->ptr_idx].aliases;
  }
  OutputExpr(std::move(op));
}

#define BACKEND_NAME BasicOptimizer
#include "calyx/backend/Templates.inl"
#pragma clang diagnostic pop

}