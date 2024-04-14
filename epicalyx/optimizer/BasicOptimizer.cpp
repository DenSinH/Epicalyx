#include "BasicOptimizer.h"
#include "RemoveUnused.h"
#include "IRCompare.h"
#include "Cast.h"
#include "Is.h"
#include "Containers.h"
#include "CustomAssert.h"
#include "Algorithm.h"


namespace epi {

using namespace calyx;

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
bool BasicOptimizer::NoBadBeforeGoodAllPaths(BadPred bad, GoodPred good, func_pos_t pos) const {
  cotyl::unordered_set<block_label_t> todo{};
  cotyl::unordered_set<block_label_t> done{};

  const auto register_branch = [&](const block_label_t& block_idx) {
    if (!done.contains(block_idx)) {
      todo.emplace(block_idx);
    }
  };

  while (true) {
    bool _reachable = true;

    // use old program blocks
    const auto& block = old_function.blocks.at(pos.first);

    while (_reachable && pos.second < block.size()) {
      const auto& op = block.at(pos.second);
      switch (op->cls) {
        case Directive::Class::Branch:
        case Directive::Class::Select: {
          auto* branch = reinterpret_cast<const Branch*>(&(*op));
          const auto destinations = branch->Destinations();
          for (const auto& block_idx : destinations) {
            register_branch(block_idx);
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

void BasicOptimizer::PropagateLocalValues() {
  // try to find local replacement from previous blocks
  auto& initial_values = local_initial_values[current_new_block_idx];

  for (const auto& [loc_idx, _] : new_function.locals) {
    local_replacement_t repl = nullptr;
    for (const auto& block_idx : old_deps.block_graph.At(current_new_block_idx).from) {
      auto new_block_idx = block_idx;
      ResolveBlockLinks(new_block_idx);
      if (!local_final_values.contains(new_block_idx)) {
        // no old value for block, can't check validity on other
        // branches, so no replacement
        // also captures the case where the target block is 
        // the current block, or loops, where the topological
        // ordering of emitting makes it so the from idx
        // block will not yet have a local_final_values
        repl = nullptr;
        break;
      }
      auto& final_values = local_final_values.at(new_block_idx);
      if (!final_values.contains(loc_idx)) {
        // no old value for local in block, can't check validity on
        // other branches, no replacement
        repl = nullptr;
        break;
      }
      auto& candidate = final_values.at(loc_idx);
      if (!repl) {
        // first replacement candidate
        repl = candidate;
      }
      else if (repl != candidate) {
        // two different candidates, skip...
        repl = nullptr;
        break;
      }
    }
    if (repl) {
      // found replacement, emit and store / propagate final value
      initial_values.insert_or_assign(loc_idx, std::move(repl));
    }
  }
}

void BasicOptimizer::StoreLocalData(var_index_t loc_idx, LocalData&& local) {
  // invalidate initial value for local in current block
  local_initial_values[current_new_block_idx].erase(loc_idx);

  // store local value
  locals.insert_or_assign(loc_idx, std::move(local));
}

bool BasicOptimizer::ShouldFlushLocal(var_index_t loc_idx, const LocalData& local) {
  const auto& old_local = old_deps.local_graph.at(loc_idx);
  cotyl::unordered_set<func_pos_t> reads  = {old_local.reads.begin(), old_local.reads.end()};
  cotyl::unordered_set<func_pos_t> writes = {old_local.writes.begin(), old_local.writes.end()};
  
  static const cotyl::unordered_set<size_t> alias_blocking_tids = {
   StoreToPointer<i8>::GetTID(),      LoadFromPointer<i8>::GetTID(),                                                                  
   StoreToPointer<u8>::GetTID(),      LoadFromPointer<u8>::GetTID(),                                                                  
   StoreToPointer<i16>::GetTID(),     LoadFromPointer<i16>::GetTID(),                                                                  
   StoreToPointer<u16>::GetTID(),     LoadFromPointer<u16>::GetTID(),                                                                      
   StoreToPointer<i32>::GetTID(),     LoadFromPointer<i32>::GetTID(),     Call<i32>::GetTID(),     CallLabel<i32>::GetTID(),     
   StoreToPointer<u32>::GetTID(),     LoadFromPointer<u32>::GetTID(),     Call<u32>::GetTID(),     CallLabel<u32>::GetTID(),     
   StoreToPointer<i64>::GetTID(),     LoadFromPointer<i64>::GetTID(),     Call<i64>::GetTID(),     CallLabel<i64>::GetTID(),     
   StoreToPointer<u64>::GetTID(),     LoadFromPointer<u64>::GetTID(),     Call<u64>::GetTID(),     CallLabel<u64>::GetTID(),     
   StoreToPointer<float>::GetTID(),   LoadFromPointer<float>::GetTID(),   Call<float>::GetTID(),   CallLabel<float>::GetTID(),       
   StoreToPointer<double>::GetTID(),  LoadFromPointer<double>::GetTID(),  Call<double>::GetTID(),  CallLabel<double>::GetTID(),        
   StoreToPointer<Struct>::GetTID(),  LoadFromPointer<Struct>::GetTID(),  Call<Pointer>::GetTID(), CallLabel<Pointer>::GetTID(),         
   StoreToPointer<Pointer>::GetTID(), LoadFromPointer<Pointer>::GetTID(), Call<Struct>::GetTID(),  CallLabel<Struct>::GetTID(),        
                                                                          Call<void>::GetTID(),    CallLabel<void>::GetTID(),      
  };

  // local does not need to be flushed if no "bad" operation (load) happens
  // before a "good" operation (store)
  return !NoBadBeforeGoodAllPaths(
    [&](const AnyDirective& op, const func_pos_t& pos) -> bool {
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
    [&](const AnyDirective& op, const func_pos_t& pos) -> bool {
      // write: stop searching path
      return writes.contains(pos);
    },
    current_old_pos
  );
}

void BasicOptimizer::FlushOnBranch() {
  auto& final_values = local_final_values[current_new_block_idx];
  for (auto& [loc_idx, local] : locals) {
    if (local.store && ShouldFlushLocal(loc_idx, local)) {
      auto store = std::move(local.store);
      Output(std::move(*store));
    }
    final_values.insert_or_assign(loc_idx, std::move(local.replacement));
  }

  // propagate any initial values that were unchanged
  for (auto& [loc_idx, repl] : local_initial_values[current_new_block_idx]) {
    if (!final_values.contains(loc_idx)) {
      final_values.emplace(loc_idx, repl);
    }
  }

  locals.clear();
}

template<typename T>
requires (std::is_base_of_v<Branch, T>)
void BasicOptimizer::DoBranch(T&& branch) {
  FlushOnBranch();
  auto destinations = branch.Destinations();
  for (const auto& dest : destinations) {
    new_block_graph.AddNodeIfNotExists(dest, nullptr);
    new_block_graph.AddEdge(current_new_block_idx, dest);
    todo.insert(dest);
  }
  
  Output(std::move(branch));
  reachable = false;
}

void BasicOptimizer::FlushAliasedLocals() {
  std::vector<var_index_t> removed{};
  auto& initial_values = local_initial_values[current_new_block_idx];
  for (auto& [loc_idx, local] : locals) {
    if (old_deps.local_graph.at(loc_idx).aliased_by.empty()) {
      continue;
    }

    // no need to check ShouldFlushLocal, 
    // this is a forced local flush
    // ShouldFlushLocal will return true anyway,
    // since this will be used on pointer writes / calls
    // if this local is aliased, but no write exists,
    // it should still be removed, as the stored read
    // value may become invalid
    removed.emplace_back(loc_idx);
    if (local.store) {
      auto store = std::move(local.store);
      Output(std::move(*store));
    }

    // erase from local_initial_values, this value is not valid
    // anymore, as an aliased local invalidating operation
    // (call, pointer write) happened
    initial_values.erase(loc_idx);
  }

  for (const auto& loc_idx : removed) {
    locals.erase(loc_idx);
  }
}

void BasicOptimizer::TryReplaceVar(var_index_t& var_idx) const {
  if (var_replacement.contains(var_idx)) {
    var_idx = var_replacement.at(var_idx);
  }
}

template<typename T>
void BasicOptimizer::TryReplaceOperand(Operand<T>& var) const {
  if (var.IsVar()) {
    TryReplaceVar(var.GetVar());
    if constexpr(!std::is_same_v<T, Struct>) {
      auto* var_imm = TryGetVarDirective<Imm<T>>(var.GetVar());
      if (var_imm) {
        var = typename Operand<T>::Imm{var_imm->value};
      }
    }
  }
}

template<typename T, class F>
bool BasicOptimizer::FindExprResultReplacement(T& op, F predicate) {
  for (const auto& [var_idx, loc] : vars_found) {
    auto& directive = new_function.blocks.at(loc.first)[loc.second];
    if (IsType<T>(directive)) {
      auto candidate_block = loc.first;
      // todo: make generic, call on new_block_graph
      auto ancestor = CommonBlockAncestor(candidate_block, current_new_block_idx);

      // todo: shift directives back for earlier ancestor blocks
      // todo: improve this
      if (ancestor == current_new_block_idx && ancestor == candidate_block) {
        const auto& candidate = directive.get<std::decay_t<T>>();
        if (predicate(candidate, op)) {
          var_replacement[op.idx] = candidate.idx;
          return true;
        }
      }
    }
  }
  return false;
}


void BasicOptimizer::ResolveBranchIndirection(block_label_t& dest) const {
  cotyl::unordered_set<block_label_t> found{dest};

  while (old_function.blocks.at(dest).size() == 1) {
    // single branch block
    // recursion resolves single branch chains
    auto& link_directive = old_function.blocks.at(dest)[0];
    if (!IsType<UnconditionalBranch>(link_directive)) {
      break;
    }
    const auto& link = link_directive.get<UnconditionalBranch>();

    dest = link.dest;
    
    // tight infinite loop of single branch blocks
    if (found.contains(dest)) break;
    found.emplace(dest);
  }
}

void BasicOptimizer::ResolveBlockLinks(block_label_t& block_idx) const {
  while (block_links.contains(block_idx)) {
    block_idx = block_links.at(block_idx).first;
  }
}

template<typename T>
const T* BasicOptimizer::TryGetVarDirective(var_index_t idx) const {
  auto [block, in_block] = vars_found.at(idx);
  auto& directive = new_function.blocks.at(block)[in_block];
  if (IsType<T>(directive)) {
    return &directive.get<T>();
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

  block_links.emplace(next_block, func_pos_t{current_new_block_idx, current_block->size()});
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

Function&& BasicOptimizer::Optimize() {
  new_function.locals = std::move(old_function.locals);

  cotyl::unordered_map<block_label_t, u32> top_sort_positions{};
  {
    auto top_sort = TopSort(old_deps.block_graph, false);
    top_sort_positions.reserve(top_sort.size());
    for (u32 i = 0; i < top_sort.size(); i++) {
      top_sort_positions[top_sort[i]] = i;
    }
  }

  todo = {Function::Entry};

  while (!todo.empty()) {
    cotyl::Assert(locals.empty());

    // emit in topological order
    {
      auto it = std::ranges::min_element(todo, [&](const block_label_t& a, const block_label_t& b) {
          return top_sort_positions.at(a) < top_sort_positions.at(b);
      });
      current_old_pos.first = current_new_block_idx = *it;
      reachable = true;
      todo.erase(it);

      // propagate new local values when branching to an
      // (ACTUAL) new block
      PropagateLocalValues();
    }

    auto inserted = new_function.blocks.emplace(current_new_block_idx, calyx::block_t{}).first;
    current_block = &inserted->second;
    auto& node = new_block_graph.EmplaceNodeIfNotExists(current_new_block_idx, nullptr);
    if (node.value) continue;  // we have already emitted this block
    node.value = current_block;

    bool block_finished;
    do {
      block_finished = true;
      const auto block = current_old_pos.first;
      current_old_pos.second = 0;
      for (const auto& directive : old_function.blocks.at(block)) {
        Emit(directive);
        // hit return statement/branch, block finished
        if (!reachable) { RemoveUnreachableBlockEdges(block); break; }
        // jumped to linked block, block NOT finished
        if (current_old_pos.first != block) { block_finished = false; break; }
        current_old_pos.second++;  
      }
    } while (!block_finished);
  }
  while (RemoveUnused(new_function));
  return std::move(new_function);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

template<typename To, typename From>
void BasicOptimizer::Emit(const Cast<To, From>& _op) {
  auto op = CopyDirective(_op);
  if constexpr(std::is_same_v<To, From>) {
    var_replacement[op.idx] = op.right_idx;
    return;
  }
  else {
    TryReplaceVar(op.right_idx);
    auto replaced = FindExprResultReplacement(op, [](auto& op, auto& candidate) {
      return candidate.right_idx == op.right_idx;
    });
    if (replaced) {
      return;
    }

    auto* right_directive = TryGetVarDirective<Imm<From>>(op.right_idx);
    if constexpr(std::is_same_v<From, Pointer>) {
      if (right_directive) {
        EmitRepl<Imm<calyx_op_type(op)::result_t>>(op.idx, (To)right_directive->value.value);
        return;
      }
    }
    else if constexpr(std::is_same_v<To, Pointer>) {
      // todo: pointer immediates
    }
    else{
      if (right_directive) {
        EmitRepl<Imm<calyx_op_type(op)::result_t>>(op.idx, (To)right_directive->value);
        return;
      }
    }
    OutputExpr(std::move(op));
  }
}

template<typename T>
void BasicOptimizer::Emit(const LoadLocal<T>& op) {
  if (locals.contains(op.loc_idx)) {
    // local value stored in IR var
    auto& repl = locals.at(op.loc_idx).replacement;
    (*repl)->idx = op.idx;
    Emit(*repl);
  }
  else {
    auto& initial_values = local_initial_values[current_new_block_idx];
    if (initial_values.contains(op.loc_idx)) {
      // propagate initial value
      auto& repl = initial_values.at(op.loc_idx);
      (*repl)->idx = op.idx;
      Emit(*repl);
    }
    else {
      // we can store this value to the locals, though without
      // a "store" directive
      if constexpr(!std::is_same_v<T, Struct>) {
        StoreLocalData(op.loc_idx, LocalData{
            .aliases = 0,
            .replacement = std::make_shared<AnyExpr>(Cast<T, calyx_op_type(op)::result_t>{0, op.idx}),
            .store = {}
        });
      }
      OutputExprCopy(op);
    }
  }
}

void BasicOptimizer::Emit(const LoadLocalAddr& op) {
  OutputExprCopy(op);
}

template<typename T>
void BasicOptimizer::Emit(const StoreLocal<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceOperand(op.src);

  if (op.src.IsVar()) {
    auto& src = op.src.GetVar();
    
    if constexpr(!std::is_same_v<T, Struct>) {
      // var alias stored to local (i.e. int var; int* al = &var; int* al2 = al;)
      // this is 0 if the "src" variable does not alias a variable anyway
      var_index_t aliases = old_deps.var_graph.at(src).aliases;

      // overwrite current local state
      const auto loc_idx = op.loc_idx;  // op will be moved when assigning
      StoreLocalData(loc_idx, LocalData{
              .aliases = aliases,
              .replacement = std::make_shared<AnyExpr>(Cast<T, calyx_op_type(op)::src_t>{0, src}),
              .store = std::make_unique<AnyDirective>(std::move(op))
      });
    }
    else {
      Output(std::move(op));
    }
  }
  else {
    const auto& src = op.src.GetImm();
    if constexpr(!std::is_same_v<T, Struct>) {
      const auto loc_idx = op.loc_idx;  // op will be moved when assigning
      StoreLocalData(loc_idx, LocalData{
              .aliases = 0,
              .replacement = std::make_shared<AnyExpr>(Imm<calyx_op_type(op)::src_t>{0, (T)src}),
              .store = std::make_unique<AnyDirective>(std::move(op))
      });
      return;
    }
    else {
      Output(std::move(op));
    }
  }
}

template<typename T>
void BasicOptimizer::Emit(const LoadGlobal<T>& op) {
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
void BasicOptimizer::Emit(const StoreGlobal<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceOperand(op.src);
  Output(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const LoadFromPointer<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.ptr_idx);
  if (old_deps.var_graph.at(op.ptr_idx).aliases) {
    // pointer aliases local variable
    auto alias = old_deps.var_graph.at(op.ptr_idx).aliases;
    if (locals.contains(alias) && op.offset == 0) {
      // IR var holds local value, emit aliased local replacement
      auto& repl = locals.at(alias).replacement;
      (*repl)->idx = op.idx;
      Emit(*repl);
    }
    else {
      // load aliased local directly
      EmitRepl<LoadLocal<T>>(op.idx, alias, op.offset);
    }
    return;
  }

  // this needs to happen, as we may be computing aliases wrong,
  // and we do not want to break the program control flow
  FlushAliasedLocals();
  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const StoreToPointer<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceOperand(op.src);
  TryReplaceVar(op.ptr_idx);
  if constexpr(!std::is_same_v<T, Struct>) {
    if (old_deps.var_graph.at(op.ptr_idx).aliases && op.offset == 0) {
      auto alias = old_deps.var_graph.at(op.ptr_idx).aliases;

      if (op.src.IsVar()) {
        auto src = op.src.GetVar();
        // storing aliased variable to another alias
        // i.e. int* zalias1 = &z; int** zaliasptr; *zaliasptr = zalias1;
        // this is 0 if the "src" variable does not alias a local anyway
        var_index_t aliases = old_deps.var_graph.at(src).aliases;

        // replace alias
        StoreLocalData(alias, LocalData{
                .aliases = aliases,
                .replacement = std::make_shared<AnyExpr>(Cast<T, calyx_op_type(op)::src_t>{0, src}),
                .store = std::make_unique<AnyDirective>(StoreLocal<T>{alias, typename Operand<calyx_op_type(op)::src_t>::Var{src}})
        });
      }
      else {
        T src = (T)op.src.GetImm();

        // replace alias
        StoreLocalData(alias, LocalData{
                .aliases = 0,
                .replacement = std::make_shared<AnyExpr>(Imm<calyx_op_type(op)::src_t>{0, src}),
                .store = std::make_unique<AnyDirective>(StoreLocal<T>{alias, typename Operand<calyx_op_type(op)::src_t>::Imm{src}})
        });
      }
      return;
    }
  }

  // need to flush local writes on pointer write
  FlushAliasedLocals();
  Output(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const Call<T>& _op) {
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

  // need to flush locals on call, in case a pointer read/store happens
  FlushAliasedLocals();
  const auto idx = op.idx;
  vars_found[idx] = Output(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const CallLabel<T>& _op) {
  auto op = CopyDirective(_op);
  for (auto& [var_idx, arg] : op.args) {
    TryReplaceVar(var_idx);
  }
  for (auto& [var_idx, arg] : op.var_args) {
    TryReplaceVar(var_idx);
  }

  // need to flush locals on call, in case a pointer read/store happens
  FlushAliasedLocals();
  // op will be moved on read
  const auto idx = op.idx;
  vars_found[idx] = Output(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const Return<T>& _op) {
  auto op = CopyDirective(_op);
  if constexpr(!std::is_same_v<T, void>) {
    TryReplaceOperand(op.val);
  }
  // no need to flush locals right before a return
  locals.clear();
  Output(std::move(op));
  reachable = false;
}

template<typename T>
void BasicOptimizer::Emit(const Imm<T>& op) {
  if constexpr(!std::is_same_v<T, Pointer>) {
    auto replaced = FindExprResultReplacement(op, [](auto& op, auto& candidate) {
      return candidate.value == op.value;
    });
    if (!replaced) {
      OutputExprCopy(op);
    }
  }
  else {
    OutputExprCopy(op);
  }
}

template<typename T>
void BasicOptimizer::Emit(const Unop<T>& _op) {
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

  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const Binop<T>& op_) {
  auto op = CopyDirective(op_);
  TryReplaceVar(op.left_idx);
  TryReplaceOperand(op.right);
  if (op.right.IsVar()) {
    auto& right_idx = op.right.GetVar();

    // fold unary expression into binop
    auto* right_unop = TryGetVarDirective<Unop<T>>(right_idx);
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

  {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      if (op.right.IsVar()) {
        switch (op.op) {
          case BinopType::Add:
          case BinopType::BinAnd:
          case BinopType::BinOr:
          case BinopType::BinXor:
          case BinopType::Mul: {
            EmitRepl<Binop<T>>(op.idx, op.right.GetVar(), op.op, typename Operand<T>::Imm{left_imm->value});
            return;
          }
          case BinopType::Sub:
          case BinopType::Div:
          case BinopType::Mod:
            // non-commutative
            break;
        }
      }
      else {
        auto right = op.right.GetImm();
        T result;
        switch (op.op) {
          case BinopType::Add: result = left_imm->value + right; break;
          case BinopType::Sub: result = left_imm->value - right; break;
          case BinopType::Mul: result = left_imm->value * right; break;
          case BinopType::Div: result = left_imm->value / right; break;
          default:
            if constexpr(is_calyx_integral_type_v<T>) {
              switch (op.op) {
                case BinopType::Mod:    result = left_imm->value % right; break;
                case BinopType::BinAnd: result = left_imm->value & right; break;
                case BinopType::BinOr:  result = left_imm->value | right; break;
                case BinopType::BinXor: result = left_imm->value ^ right; break;
                default: break;
              }
            }
            break;
        }
        EmitRepl<Imm<T>>(op.idx, result);
        return;
      }
    }

    if (op.right.IsVar()) {
      // fold unop into binop
      auto* left_unop = TryGetVarDirective<Unop<T>>(op.left_idx);
      if (left_unop) {
        if (left_unop->op == UnopType::Neg) {
          if (op.op == BinopType::Add) {
            EmitRepl<Binop<T>>(op.idx, op.right.GetVar(), BinopType::Sub, left_unop->right_idx);
            return;
          }
        }
      }
    }
    else {
      // result (v2 = (v1 @second b) @first a
      auto* left_binim = TryGetVarDirective<Binop<T>>(op.left_idx);
      if (left_binim && left_binim->right.IsImm()) {
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
          EmitRepl<Binop<T>>(op.idx, left_binim->left_idx, repl_op, typename Operand<T>::Imm{repl_fn(op.right.GetImm(), left_binim->right.GetImm())});
          return;
        }
      }
    }
  }

  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const Shift<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceOperand(op.left);
  TryReplaceOperand(op.right);

  if (op.right.IsImm()) {
    if (op.left.IsImm()) {
      // both are imm
      T result;
      switch (op.op) {
        case ShiftType::Left:  result = op.left.GetImm() << op.right.GetImm(); break;
        case ShiftType::Right: result = op.left.GetImm() >> op.right.GetImm(); break;
      }
      EmitRepl<Imm<T>>(op.idx, result);
      return;
    }
    else if (op.right.GetImm() == 0) {
      var_replacement[op.idx] = op.left.GetVar();
      return;
    }
  }

  OutputExpr(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const Compare<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.left_idx);
  TryReplaceOperand(op.right);

  // replace left side
  if constexpr(!std::is_same_v<T, Pointer>) {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      if (op.right.IsImm()) {
        auto right = op.right.GetImm();
        i32 result;
        
        switch (op.op) {
          case CmpType::Eq: result = left_imm->value == right; break;
          case CmpType::Ne: result = left_imm->value != right; break;
          case CmpType::Lt: result = left_imm->value <  right; break;
          case CmpType::Le: result = left_imm->value <= right; break;
          case CmpType::Gt: result = left_imm->value >  right; break;
          case CmpType::Ge: result = left_imm->value >= right; break;
        }
        EmitRepl<Imm<i32>>(op.idx, result);
        return;
      }
      else {
        CmpType flipped = op.op;

        switch (op.op) {
          case CmpType::Eq:
          case CmpType::Ne: break;
          case CmpType::Lt: flipped = CmpType::Gt; break;
          case CmpType::Le: flipped = CmpType::Ge; break;
          case CmpType::Gt: flipped = CmpType::Lt; break;
          case CmpType::Ge: flipped = CmpType::Le; break;
        }
        EmitRepl<Compare<T>>(op.idx, op.right.GetVar(), flipped, typename Operand<T>::Imm{left_imm->value});
        return;
      }
    }

    if (op.right.IsImm()) {
      auto& right = op.right.GetImm();
      // replace comparisons with added constants
      // i.e., change (left + 1) > 0 with left > -1
      auto* left_binim = TryGetVarDirective<Binop<T>>(op.left_idx);
      if (left_binim && left_binim->right.IsImm()) {
        auto binim_right = left_binim->right.GetImm();
        switch (left_binim->op) {
          case BinopType::Add:
            op.left_idx = left_binim->left_idx;
            right -= binim_right;
            break;
          case BinopType::Sub:
            op.left_idx = left_binim->left_idx;
            right += binim_right;
            break;
          case BinopType::BinXor: {
            if constexpr(std::is_integral_v<T>) {
              if (op.op == CmpType::Eq || op.op == CmpType::Ne) {
                op.left_idx = left_binim->left_idx;
                right ^= binim_right;
              }
            }
            break;
          }
          default:
            break;
        }
      }
    }
  }

  OutputExpr(std::move(op));
}

void BasicOptimizer::Emit(const UnconditionalBranch& _op) {
  auto op = CopyDirective(_op);
  ResolveBranchIndirection(op.dest);
  const auto& block_deps = new_block_graph.At(current_new_block_idx);
  if (block_deps.to.empty()) {
    // no dependencies so far, we can link this block if the next block only has one input
    bool can_link = !new_block_graph.Has(op.dest) 
      && old_deps.block_graph.At(op.dest).from.size() == 1
      && *old_deps.block_graph.At(op.dest).from.begin() == current_old_pos.first;
    if (can_link) {
      LinkBlock(op.dest);
      return;
    }
  }

  DoBranch(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const BranchCompare<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.left_idx);
  TryReplaceOperand(op.right);

  // replace left side
  // not both sides can be constants, otherwise they would have been folded already
  if constexpr(!std::is_same_v<T, Pointer>) {
    auto* left_imm = TryGetVarDirective<Imm<T>>(op.left_idx);
    if (left_imm) {
      if (op.right.IsVar()) {
        CmpType flipped = op.op;

        // note: flipped, not converse, we are simply chainging
        // left < right with right > left for example
        switch (op.op) {
          case CmpType::Eq:
          case CmpType::Ne: break;
          case CmpType::Lt: flipped = CmpType::Gt; break;
          case CmpType::Le: flipped = CmpType::Ge; break;
          case CmpType::Gt: flipped = CmpType::Lt; break;
          case CmpType::Ge: flipped = CmpType::Le; break;
        }
        EmitRepl<BranchCompare<T>>(op.tdest, op.fdest, op.right.GetVar(), flipped, typename Operand<T>::Imm{left_imm->value});
        return;
      }
      else {
        auto right = op.right.GetImm();
        auto emit_unconditional = [&](bool cmp) {
          if (cmp) EmitRepl<UnconditionalBranch>(op.tdest);
          else     EmitRepl<UnconditionalBranch>(op.fdest);
        };

        switch (op.op) {
          case CmpType::Eq: emit_unconditional(left_imm->value == right); break;
          case CmpType::Ne: emit_unconditional(left_imm->value != right); break;
          case CmpType::Lt: emit_unconditional(left_imm->value <  right); break;
          case CmpType::Le: emit_unconditional(left_imm->value <= right); break;
          case CmpType::Gt: emit_unconditional(left_imm->value >  right); break;
          case CmpType::Ge: emit_unconditional(left_imm->value >= right); break;
        }
        return;
      }
    }
  }

  if constexpr(!std::is_same_v<T, Pointer>) {
    if (op.right.IsImm()) {
      auto& right = op.right.GetImm();
      // replace comparisons with added constants
      // i.e., change (left + 1) > 0 with left > -1
      auto* left_binim = TryGetVarDirective<Binop<T>>(op.left_idx);
      if (left_binim && left_binim->right.IsImm()) {
        auto binim_right = left_binim->right.GetImm();
        switch (left_binim->op) {
          case BinopType::Add:
            op.left_idx = left_binim->left_idx;
            right -= binim_right;
            break;
          case BinopType::Sub:
            op.left_idx = left_binim->left_idx;
            right += binim_right;
            break;
          case BinopType::BinXor: {
            if constexpr(std::is_integral_v<T>) {
              if (op.op == CmpType::Eq || op.op == CmpType::Ne) {
                op.left_idx = left_binim->left_idx;
                right ^= binim_right;
              }
            }
            break;
          }
          default:
            break;
        }
      }
    }
  }

  ResolveBranchIndirection(op.tdest);
  ResolveBranchIndirection(op.fdest);
  DoBranch(std::move(op));
}

void BasicOptimizer::Emit(const Select& _op) {
  auto op = CopyDirective(_op);
  TryReplaceVar(op.idx);

  auto* val_imm = TryGetVarDirective<Imm<calyx_op_type(op)::src_t>>(op.idx);
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

  for (auto& [val, block_idx] : op.table) {
    ResolveBranchIndirection(block_idx);
  }
  if (op._default) {
    ResolveBranchIndirection(op._default);
  }

  DoBranch(std::move(op));
}

template<typename T>
void BasicOptimizer::Emit(const AddToPointer<T>& _op) {
  auto op = CopyDirective(_op);
  TryReplaceOperand(op.ptr);
  TryReplaceOperand(op.right);

  if (op.right.IsImm()) {
    // todo: if (op.left.IsImm())
    auto right = op.right.GetImm();
    if (right == 0) {
      if (op.ptr.IsVar()) var_replacement[op.idx] = op.ptr.GetVar();
      else EmitRepl<Imm<Pointer>>(op.idx, op.ptr.GetImm());
      return;
    }
  }
  
  OutputExpr(std::move(op));
}

}