#include "ASTWalker.h"
#include "Emitter.h"
#include "Helpers.h"

#include "types/Types.h"
#include "calyx/Directive.h"
#include "ast/Statement.h"
#include "ast/Declaration.h"
#include "ast/Expression.h"

#include "Exceptions.h"
#include "CustomAssert.h"

#include "Helpers.inl"


namespace epi {

using namespace ast;

void ASTWalker::Visit(EmptyNode& stat) {

}

void ASTWalker::Visit(IfNode& stat) {
  auto true_block = emitter.MakeBlock();
  auto false_block = emitter.MakeBlock();
  block_label_t post_block;

  state.push({State::ConditionalBranch, {.true_block = true_block, .false_block = false_block}});
  stat.cond->Visit(*this);
  state.pop();

  // create post block here to improve optimization ancestry finding
  if (stat._else) {
    post_block = emitter.MakeBlock();
  }
  else {
    post_block = false_block;
  }
  // true, then jump to post block
  // emitter should have emitted branches to blocks
  emitter.SelectBlock(true_block);
  stat.stat->Visit(*this);
  emitter.Emit<calyx::UnconditionalBranch>(post_block);

  if (stat._else) {
    emitter.SelectBlock(false_block);
    stat._else->Visit(*this);
    emitter.Emit<calyx::UnconditionalBranch>(post_block);
  }

  // create post block here to improve optimization ancestry finding
  emitter.SelectBlock(post_block);
}

void ASTWalker::Visit(WhileNode& stat) {
  // loop entry is lowest index
  auto cond_block = emitter.MakeBlock();
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);
  emitter.SelectBlock(cond_block);

  auto loop_block = emitter.MakeBlock();
  auto post_block = emitter.MakeBlock();

  state.push({State::ConditionalBranch, {.true_block = loop_block, .false_block = post_block}});
  stat.cond->Visit(*this);
  state.pop();

  // branching to blocks should have been emitted by the cond Visit
  emitter.SelectBlock(loop_block);

  break_stack.push(post_block);
  continue_stack.push(cond_block);

  stat.stat->Visit(*this);

  break_stack.pop();
  continue_stack.pop();

  // loop back to the condition block
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);

  // go to block after loop
  emitter.SelectBlock(post_block);
}

void ASTWalker::Visit(DoWhileNode& stat) {
  // loop entry is lowest index
  auto loop_block = emitter.MakeBlock();
  auto cond_block = emitter.MakeBlock();
  auto post_block = emitter.MakeBlock();

  // branch to loop block always happens at least once
  emitter.Emit<calyx::UnconditionalBranch>(loop_block);
  emitter.SelectBlock(loop_block);

  break_stack.push(post_block);
  continue_stack.push(cond_block);

  stat.stat->Visit(*this);

  break_stack.pop();
  continue_stack.pop();

  // go to cond block
  // this is a separate block for continue statements
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);
  emitter.SelectBlock(cond_block);

  state.push({State::ConditionalBranch, {.true_block = loop_block, .false_block = post_block}});
  stat.cond->Visit(*this);
  state.pop();
  // branching to loop back to the loop block should have been handled by condition

  // go to block after loop
  emitter.SelectBlock(post_block);
}

void ASTWalker::Visit(ForNode& stat) {
  
  auto init_block = emitter.MakeBlock();

  // new scope for declarations in for loop
  locals.NewLayer();

  // always go to initialization
  emitter.Emit<calyx::UnconditionalBranch>(init_block);
  emitter.SelectBlock(init_block);
  for (auto& decl : stat.decls) {
    Visit(decl);
  }
  for (auto& init : stat.inits) {
    init->Visit(*this);
  }

  // loop entry is lowest block
  auto cond_block = emitter.MakeBlock();
  auto loop_block = emitter.MakeBlock();
  auto post_block = emitter.MakeBlock();
  // go to condition
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);
  emitter.SelectBlock(cond_block);

  state.push({State::ConditionalBranch, {.true_block = loop_block, .false_block = post_block}});
  stat.cond->Visit(*this);
  state.pop();

  auto update_block = emitter.MakeBlock();
  // branching case should have been handled by condition
  emitter.SelectBlock(loop_block);

  break_stack.push(post_block);
  continue_stack.push(update_block);

  // loop body
  stat.stat->Visit(*this);

  break_stack.pop();
  continue_stack.pop();

  // go to update and loop back to condition
  emitter.Emit<calyx::UnconditionalBranch>(update_block);
  emitter.SelectBlock(update_block);
  for (auto& update : stat.updates) {
    update->Visit(*this);
  }
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);

  // pop locals layer
  locals.PopLayer();

  // go to block after loop
  emitter.SelectBlock(post_block);
}

void ASTWalker::Visit(LabelNode& stat) {
  if (!local_labels.contains(stat.name)) {
    auto block = emitter.MakeBlock();
    local_labels.emplace(stat.name, block);
    emitter.Emit<calyx::UnconditionalBranch>(block);
    emitter.SelectBlock(block);
  }
  else {
    auto block = local_labels.at(stat.name);
    emitter.Emit<calyx::UnconditionalBranch>(block);
    emitter.SelectBlock(block);
  }
  stat.stat->Visit(*this);
}

void ASTWalker::Visit(SwitchNode& stat) {
  state.push({State::Read, {}});
  stat.expr->Visit(*this);
  state.pop();

  auto right = current;
  switch (emitter.vars[right].type) {
    case Emitter::Var::Type::I32: {
      right = emitter.EmitExpr<calyx::Cast<i64, i32>>({ Emitter::Var::Type::U32 }, right);
      break;
    }
    case Emitter::Var::Type::U32: {
      right = emitter.EmitExpr<calyx::Cast<i64, u32>>({ Emitter::Var::Type::U32 }, right);
      break;
    }
    case Emitter::Var::Type::U64: {
      right = emitter.EmitExpr<calyx::Cast<i64, u64>>({ Emitter::Var::Type::U32 }, right);
      break;
    }
    case Emitter::Var::Type::I64: break;
    default: {
      throw std::runtime_error("Bad operand type for switch statement");
    }
  }

  auto select = emitter.Emit<calyx::Select>(right);
  auto post_block = emitter.MakeBlock();

  if (select) {
    // we might be in an unreachable spot
    break_stack.push(post_block);
    select_stack.push(select);
    stat.stat->Visit(*this);
    select_stack.pop();
    break_stack.pop();
  }

  emitter.Emit<calyx::UnconditionalBranch>(post_block);
  emitter.SelectBlock(post_block);
}

void ASTWalker::Visit(CaseNode& stat) {
  cotyl::Assert(!select_stack.empty(), "Invalid case statement");
  auto* select = select_stack.top();
  cotyl::Assert(!select->table->contains(stat.expr), "Duplicate case statement");

  auto block = emitter.MakeBlock();
  // check for fallthrough and don't emit branch if there is no fallthrough
  emitter.Emit<calyx::UnconditionalBranch>(block);

  select->table->emplace(stat.expr, block);

  emitter.SelectBlock(block);
  stat.stat->Visit(*this);
}

void ASTWalker::Visit(DefaultNode& stat) {
  cotyl::Assert(!select_stack.empty(), "Invalid default statement");
  auto* select = select_stack.top();
  cotyl::Assert(select->_default == 0, "Duplicate default statement");

  auto block = emitter.MakeBlock();
  // check for fallthrough and don't emit branch if there is no fallthrough
  emitter.Emit<calyx::UnconditionalBranch>(block);

  select->_default = block;

  emitter.SelectBlock(block);
  stat.stat->Visit(*this);
}

void ASTWalker::Visit(GotoNode& stat) {
  if (!local_labels.contains(stat.label)) {
    auto block = emitter.MakeBlock();
    local_labels.emplace(stat.label, block);
    emitter.Emit<calyx::UnconditionalBranch>(block);
  }
  else {
    emitter.Emit<calyx::UnconditionalBranch>(local_labels.at(stat.label));
  }
}

void ASTWalker::Visit(ReturnNode& stat) {
  if (stat.expr) {
    state.push({State::Read, {}});
    stat.expr->Visit(*this);
    auto visitor = detail::EmitterTypeVisitor<detail::ReturnEmitter>(*this, { current });
    visitor.Visit(*function->signature.contained);
    state.pop();
  }
  else {
    emitter.Emit<calyx::Return<void>>();
  }
}

void ASTWalker::Visit(BreakNode& stat) {
  cotyl::Assert(!break_stack.empty());
  emitter.Emit<calyx::UnconditionalBranch>(break_stack.top());
}

void ASTWalker::Visit(ContinueNode& stat) {
  cotyl::Assert(!continue_stack.empty());
  emitter.Emit<calyx::UnconditionalBranch>(continue_stack.top());
}

void ASTWalker::Visit(CompoundNode& stat) {
  locals.NewLayer();
  for (const auto& node : stat.stats) {
    node->Visit(*this);
  }
  locals.PopLayer();
}

}
