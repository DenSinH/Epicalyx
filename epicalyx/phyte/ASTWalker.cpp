#include "ASTWalker.h"
#include "Emitter.h"
#include "calyx/Calyx.h"
#include "types/EpiCType.h"
#include "taxy/Declaration.h"
#include "taxy/Statement.h"
#include "taxy/Expression.h"
#include "Is.h"

#include "Assert.h"

#include "Helpers.inl"


namespace epi::phyte {

void ASTWalker::Visit(epi::taxy::Declaration& decl) {
  if (variables.Depth() == 1) {
    // global symbols
    u64 size = decl.type->Sizeof();
    emitter.program.globals[decl.name] = size;
    c_types.Set(decl.name, decl.type);
    if (decl.value.has_value()) {
      if (std::holds_alternative<pExpr>(decl.value.value())) {
        auto& expr = std::get<pExpr>(decl.value.value());

        auto block = emitter.MakeBlock();
        emitter.SelectBlock(block);

        state.push({State::Read, {}});
        expr->Visit(*this);
        auto visitor = detail::EmitterTypeVisitor<detail::ReturnEmitter>(*this, { current });
        decl.type->Visit(visitor);
        state.pop();

        emitter.program.global_init[decl.name] = block;
      }
      else {
        // todo: handle initializer list
        throw std::runtime_error("Unimplemented: global initializer list declaration");
      }
    }
  }
  else {
    auto c_idx = emitter.c_counter++;
    u64 size = decl.type->Sizeof();
    variables.Set(decl.name, calyx::CVar{
            c_idx, calyx::CVar::Location::Either, size
    });
    c_types.Set(decl.name, decl.type);
    emitter.Emit<calyx::AllocateLocal>(c_idx, size);

    if (decl.value.has_value()) {
      if (std::holds_alternative<pExpr>(decl.value.value())) {
        state.push({State::Read, {}});
        std::get<pExpr>(decl.value.value())->Visit(*this);
        state.pop();
        // current now holds the expression id that we want to assign with
        state.push({State::Assign, {.var = current}});
        Identifier(decl.name).Visit(*this);
        state.pop();
      }
      else {
        // todo: handle initializer list
        throw std::runtime_error("Unimplemented: initializer list declaration");
      }
    }
  }
}

void ASTWalker::Visit(epi::taxy::FunctionDefinition& decl) {
  // same as normal compound statement besides arguments
  variables.NewLayer();
  c_types.NewLayer();

  // todo: arguments etc
  // after arguments add another scope
  function = &decl;
  auto block = emitter.MakeBlock();
  emitter.SelectBlock(block);
  emitter.program.functions.emplace(decl.symbol, block);
  for (const auto& node : decl.body->stats) {
    node->Visit(*this);
  }
  for (auto var = variables.Top().rbegin(); var != variables.Top().rend(); var++) {
    emitter.Emit<calyx::DeallocateLocal>(var->second.idx, var->second.size);
  }
  variables.PopLayer();
  c_types.NewLayer();
}

void ASTWalker::Visit(Identifier& decl) {
  // after the AST, the only identifiers left are C variables
  if (state.empty()) {
    // statement has no effect
    return;
  }

  if (state.top().first == State::ConditionalBranch) {
    // this makes no difference for local / global symbols

    // first part is same as read
    state.push({State::Read, {}});
    Visit(decl);
    state.pop();

    // emit branch
    // branch to false block if 0, otherwise go to true block
    auto block = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, block, current, calyx::CmpType::Eq, 0);
    emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
    return;
  }
  
  auto type = c_types.Get(decl.name);
  if (variables.Has(decl.name)) {
    // local variable
    auto cvar = variables.Get(decl.name);
    switch (state.top().first) {
      case State::Read: {
        if (type->IsArray()) {
          current = emitter.EmitExpr<calyx::LoadLocalAddr>({calyx::Var::Type::Pointer, type->Deref()->Sizeof() }, cvar.idx);
        }
        else {
          auto visitor = detail::EmitterTypeVisitor<detail::LoadLocalEmitter>(*this, {cvar.idx});
          type->Visit(visitor);
        }
        break;
      }
      case State::Assign: {
        auto visitor = detail::EmitterTypeVisitor<detail::StoreLocalEmitter>(
                *this, { cvar.idx, state.top().second.var }
        );
        type->Visit(visitor);
        break;
      }
      case State::Address: {
        // we can't get the address of a variable that is not on the stack
        variables.Get(decl.name).loc = calyx::CVar::Location::Stack;
        auto visitor = detail::EmitterTypeVisitor<detail::LoadLocalAddrEmitter>(*this, {cvar.idx});
        type->Visit(visitor);
        break;
      }
      default: {
        throw std::runtime_error("Bad declaration state");
      }
    }
  }
  else {
    // global symbol
    switch (state.top().first) {
      case State::Read: {
        if (type->IsArray()) {
          current = emitter.EmitExpr<calyx::LoadGlobalAddr>({calyx::Var::Type::Pointer, type->Deref()->Sizeof() }, decl.name);
        }
        else {
          auto visitor = detail::EmitterTypeVisitor<detail::LoadGlobalEmitter>(*this, {decl.name});
          type->Visit(visitor);
        }
        break;
      }
      case State::Assign: {
        auto visitor = detail::EmitterTypeVisitor<detail::StoreGlobalEmitter>(
                *this, { decl.name, state.top().second.var }
        );
        type->Visit(visitor);
        break;
      }
      case State::Address: {
        auto visitor = detail::EmitterTypeVisitor<detail::LoadGlobalAddrEmitter>(*this, {decl.name});
        type->Visit(visitor);
        break;
      }
      default: {
        throw std::runtime_error("Bad declaration state");
      }
    }
  }
}

template<typename T>
void ASTWalker::ConstVisitImpl(NumericalConstant<T>& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  if (state.top().first == State::ConditionalBranch) {
    if (expr.value) {
      emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
    }
    else {
      emitter.Emit<calyx::UnconditionalBranch>(state.top().second.false_block);
    }
  }
  else {
    current = emitter.EmitExpr<calyx::Imm<calyx::calyx_upcast_t<T>>>({ detail::calyx_type_v<calyx::calyx_upcast_t<T>> }, expr.value);
  }
}

template void ASTWalker::ConstVisitImpl(NumericalConstant<i8>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<u8>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<i16>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<u16>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<i32>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<u32>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<i64>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<u64>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<float>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<double>&);

void ASTWalker::Visit(StringConstant& expr) {
  // load from rodata
  throw std::runtime_error("unimplemented: string constant");
}

void ASTWalker::Visit(ArrayAccess& expr) {
  cotyl::Assert(state.empty() || cotyl::Is(state.top().first).AnyOf<State::Read, State::ConditionalBranch, State::Assign, State::Address>());

  state.push({State::Read, {}});
  calyx::var_index_t ptr_idx, offs_idx;
  if (expr.left->GetType()->IsPointer()) {
    expr.left->Visit(*this);
    ptr_idx = current;
    expr.right->Visit(*this);
    offs_idx = current;
  }
  else {
    expr.right->Visit(*this);
    ptr_idx = current;
    expr.left->Visit(*this);
    offs_idx = current;
  }
  state.pop();

  auto ptr_var = emitter.vars[ptr_idx];
  EmitPointerIntegralExpr<calyx::AddToPointer>(
          emitter.vars[offs_idx].type, ptr_var.stride, ptr_idx, calyx::PtrAddType::Add, ptr_var.stride, offs_idx
  );
  ptr_idx = current;

  if (state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch) {
    auto visitor = detail::EmitterTypeVisitor<detail::LoadFromPointerEmitter>(*this, { ptr_idx });
    if (expr.left->GetType()->IsPointer()) {
      expr.left->GetType()->Deref()->Visit(visitor);
    }
    else {
      expr.right->GetType()->Deref()->Visit(visitor);
    }

    if (!state.empty() && state.top().first == State::ConditionalBranch) {
      auto false_block = state.top().second.false_block;
      EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, false_block, current, calyx::CmpType::Eq, 0);
      emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
    }
  }
  else {
    switch (state.top().first) {
      case State::Assign: {
        auto var = state.top().second.var;
        auto visitor = detail::EmitterTypeVisitor<detail::StoreToPointerEmitter>(*this, { ptr_idx, var });
        if (expr.left->GetType()->IsPointer()) {
          expr.left->GetType()->Deref()->Visit(visitor);
        }
        else {
          expr.right->GetType()->Deref()->Visit(visitor);
        }

        // stored value is "returned" value
        current = var;
        break;
      }
      case State::Address: {
        // current now holds the address already
        break;
      }
      default: {
        throw std::runtime_error("Bad state");
      }
    }
  }
}

void ASTWalker::Visit(FunctionCall& expr) {
  throw std::runtime_error("unimplemented: function call");
}

void ASTWalker::Visit(MemberAccess& expr) {
  throw std::runtime_error("unimplemented: member access");
}

void ASTWalker::Visit(TypeInitializer& expr) {
  throw std::runtime_error("unimplemented: type initializer");
}

void ASTWalker::Visit(PostFix& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  
  bool conditional_branch = !state.empty() && state.top().first == State::ConditionalBranch;

  switch (expr.op) {
    case TokenType::Incr: {
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();
      auto read = current;
      auto type = emitter.vars[current].type;
      if (type == calyx::Var::Type::Pointer) {
        auto var = emitter.vars[read];
        current = emitter.EmitExpr<calyx::AddToPointerImm>(var, read, var.stride, 1);
      }
      else if (type == calyx::Var::Type::Struct) {
        throw std::runtime_error("Bad expression for post-increment: struct");
      }
      else {
        EmitArithExpr<calyx::BinopImm>(type, read, calyx::BinopType::Add, 1);
      }

      // write back
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();
      // restore read value for next expression
      current = read;

      // need to check for conditional branches
      break;
    }
    case TokenType::Decr: {
      cotyl::Assert(state.empty() || state.top().first == State::Read);
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();
      auto read = current;
      auto type = emitter.vars[current].type;
      if (type == calyx::Var::Type::Pointer) {
        auto var = emitter.vars[read];
        current = emitter.EmitExpr<calyx::AddToPointerImm>(var, read, var.stride, -1);
      }
      else if (type == calyx::Var::Type::Struct) {
        throw std::runtime_error("Bad expression for post-decrement: struct");
      }
      else {
        EmitArithExpr<calyx::BinopImm>(type, read, calyx::BinopType::Sub, 1);
      }

      // write back
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();
      // restore read value for next expression
      current = read;

      // need to check for conditional branches
      break;
    }
    default: {
      throw std::runtime_error("Bad postfix");
    }
  }

  if (conditional_branch) {
    auto false_block = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, false_block, current, calyx::CmpType::Eq, 0);
    emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
  }
}

void ASTWalker::Visit(Unary& expr) {
  const bool conditional_branch = !state.empty() && state.top().first == State::ConditionalBranch;

  switch (expr.op) {
    case TokenType::Minus: {
      cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      // no need to push a new state
      expr.left->Visit(*this);
      if (!conditional_branch) {
        EmitArithExpr<calyx::Unop>(emitter.vars[current].type, calyx::UnopType::Neg, current);
      }
      // return, no need to check for conditional branches, is handled in visiting the expr
      return;
    }
    case TokenType::Plus: {
      cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      // no need to push a new state
      // does nothing
      // return, no need to check for conditional branches, is handled in visiting the expr
      expr.left->Visit(*this);
      return;
    }
    case TokenType::Tilde: {
      cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      // no need to push a new state
      expr.left->Visit(*this);
      EmitArithExpr<calyx::Unop>(emitter.vars[current].type, calyx::UnopType::BinNot, current);
      // break, need to check for conditional branches
      break;
    }
    case TokenType::Exclamation: {
      cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      if (conditional_branch) {
        auto false_block = state.top().second.false_block;
        // branch to false if current != 0 (if current == 0, then !current is true)
        EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, false_block, current, calyx::CmpType::Ne, 0);
        emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
      }
      else {
        EmitCompare<calyx::CompareImm>(emitter.vars[current].type, current, calyx::CmpType::Eq, 0);
      }

      // return, no need to check for conditional branches, already handled
      return;
    }
    case TokenType::Incr: {
      cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      auto type = emitter.vars[current].type;
      calyx::var_index_t stored;
      if (type == calyx::Var::Type::Pointer) {
        auto var = emitter.vars[current];
        current = emitter.EmitExpr<calyx::AddToPointerImm>(var, current, var.stride, 1);
      }
      else if (type == calyx::Var::Type::Struct) {
        throw std::runtime_error("Bad expression for pre-increment: struct");
      }
      else {
        EmitArithExpr<calyx::BinopImm>(type, current, calyx::BinopType::Add, 1);
      }
      stored = current;

      // write back
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();

      // restore stored value
      current = stored;
      // break, need to check for conditional branches
      break;
    }
    case TokenType::Decr: {
      cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      // read value
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      auto type = emitter.vars[current].type;
      calyx::var_index_t stored;

      // subtract 1
      if (type == calyx::Var::Type::Pointer) {
        auto var = emitter.vars[current];
        current = emitter.EmitExpr<calyx::AddToPointerImm>(var, current, var.stride, -1);
      }
      else if (type == calyx::Var::Type::Struct) {
        throw std::runtime_error("Bad expression for pre-decrement: struct");
      }
      else {
        EmitArithExpr<calyx::BinopImm>(type, current, calyx::BinopType::Sub, 1);
      }
      stored = current;

      // write back
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();

      // restore stored value
      current = stored;

      // break, need to check for conditional branches
      break;
    }
    case TokenType::Ampersand: {
      cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      state.push({ State::Address, {} });
      expr.left->Visit(*this);
      state.pop();
      break;
    }
    case TokenType::Asterisk: {
      cotyl::Assert(state.empty() || cotyl::Is(state.top().first).AnyOf<State::Read, State::ConditionalBranch, State::Assign, State::Address>());
      if (!state.empty() && state.top().first == State::Assign) {
        state.push({ State::Read, {} });
        expr.left->Visit(*this);
        state.pop();

        auto var = state.top().second.var;
        auto visitor = detail::EmitterTypeVisitor<detail::StoreToPointerEmitter>(*this, { current, var });
        expr.left->GetType()->Deref()->Visit(visitor);

        // stored value is "returned" value
        current = var;
        return;
      }
      else if (!state.empty() && state.top().first == State::Address) {
        // &(*pointer) == pointer
        state.push({ State::Read, {} });
        expr.left->Visit(*this);
        state.pop();
        // no need to check for conditional branch
        return;
      }
      else {
        state.push({ State::Read, {} });
        expr.left->Visit(*this);
        state.pop();

        auto visitor = detail::EmitterTypeVisitor<detail::LoadFromPointerEmitter>(*this, { current });
        expr.left->GetType()->Deref()->Visit(visitor);

        // could be a conditional branch
      }
      break;
    }
    default: {
      throw std::runtime_error("Bad unop");
    }
  }

  if (conditional_branch) {
    auto false_block = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, false_block, current, calyx::CmpType::Eq, 0);
    emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
  }
}

void ASTWalker::Visit(Cast& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  // no need to push a new state
  expr.expr->Visit(*this);

  // only need to emit an actual cast if we are not checking for 0
  if (state.empty() || state.top().first != State::ConditionalBranch) {
    auto visitor = detail::EmitterTypeVisitor<detail::CastToEmitter>(*this, { current });
    expr.type->Visit(visitor);
  }
}

void ASTWalker::Visit(Binop& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  
  // only need to push a new state for conditional branches
  const bool conditional_branch = !state.empty() && (state.top().first == State::ConditionalBranch);
  if (conditional_branch) {
    state.push({State::Read, {}});
  }
  expr.left->Visit(*this);
  auto left = current;
  expr.right->Visit(*this);
  auto right = current;
  if (conditional_branch) {
    state.pop();
  }

  switch (expr.op) {
    case TokenType::Asterisk: BinopHelper(left, calyx::BinopType::Mul, right); break;
    case TokenType::Div: BinopHelper(left, calyx::BinopType::Div, right); break;
    case TokenType::Mod: BinopHelper(left, calyx::BinopType::Mod, right); break;
    case TokenType::Ampersand: BinopHelper(left, calyx::BinopType::BinAnd, right); break;
    case TokenType::BinOr: BinopHelper(left, calyx::BinopType::BinOr, right); break;
    case TokenType::BinXor: BinopHelper(left, calyx::BinopType::BinXor, right); break;
    case TokenType::Plus: {
      if (emitter.vars[left].type == calyx::Var::Type::Pointer) {
        auto var = emitter.vars[left];
        EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[right].type, var.stride, left, calyx::PtrAddType::Add, var.stride, right);
      }
      else if (emitter.vars[right].type == calyx::Var::Type::Pointer) {
        auto var = emitter.vars[right];
        EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[left].type, var.stride, right, calyx::PtrAddType::Add, var.stride, left);
      }
      else {
        BinopHelper(left, calyx::BinopType::Add, right);
      }
      break;
    }
    case TokenType::Minus: {
      if (emitter.vars[left].type == calyx::Var::Type::Pointer) {
        auto var = emitter.vars[left];
        if (emitter.vars[right].type == calyx::Var::Type::Pointer) {
          throw std::runtime_error("Uniplemented: pointer diff");
        }
        else {
          EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[right].type, var.stride, left, calyx::PtrAddType::Sub, var.stride, right);
        }
      }
      else if (emitter.vars[right].type == calyx::Var::Type::Pointer) {
        throw std::runtime_error("Invalid right hand side operator for -: pointer");
      }
      else {
        BinopHelper(left, calyx::BinopType::Sub, right);
      }
      break;
    }
    case TokenType::Less:
    case TokenType::LessEqual:
    case TokenType::GreaterEqual:
    case TokenType::Greater:
    case TokenType::Equal:
    case TokenType::NotEqual: {
      auto casted = BinopCastHelper(left, right);
      calyx::CmpType cmp_type;
      calyx::CmpType inverse;
      switch (expr.op) {
        case TokenType::Less:
          cmp_type = calyx::CmpType::Lt;
          inverse = calyx::CmpType::Ge;
          break;
        case TokenType::LessEqual:
          cmp_type = calyx::CmpType::Le;
          inverse = calyx::CmpType::Gt;
          break;
        case TokenType::GreaterEqual:
          cmp_type = calyx::CmpType::Ge;
          inverse = calyx::CmpType::Lt;
          break;
        case TokenType::Greater:
          cmp_type = calyx::CmpType::Gt;
          inverse = calyx::CmpType::Le;
          break;
        case TokenType::Equal:
          cmp_type = calyx::CmpType::Eq;
          inverse = calyx::CmpType::Ne;
          break;
        case TokenType::NotEqual:
          cmp_type = calyx::CmpType::Ne;
          inverse = calyx::CmpType::Eq;
          break;
        default:
          throw std::runtime_error("Unreachable");
      }

      if (conditional_branch) {
        auto false_block = state.top().second.false_block;
        EmitBranch<calyx::BranchCompare>(casted.var.type, false_block, casted.left, inverse, casted.right);
        emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
      }
      else {
        EmitCompare<calyx::Compare>(casted.var.type, casted.left, cmp_type, casted.right);
      }
      // no need to check for conditional branching after this
      return;
    }
    case TokenType::LShift:
    case TokenType::RShift: {
      cotyl::Assert(!cotyl::Is(emitter.vars[left].type).AnyOf<calyx::Var::Type::Float, calyx::Var::Type::Double, calyx::Var::Type::Pointer, calyx::Var::Type::Struct>());
      cotyl::Assert(!cotyl::Is(emitter.vars[right].type).AnyOf<calyx::Var::Type::Float, calyx::Var::Type::Double, calyx::Var::Type::Pointer, calyx::Var::Type::Struct>());
      switch (emitter.vars[right].type) {
        case calyx::Var::Type::I32: {
          right = emitter.EmitExpr<calyx::Cast<u32, i32>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::I64: {
          right = emitter.EmitExpr<calyx::Cast<u32, i64>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::U64: {
          right = emitter.EmitExpr<calyx::Cast<u32, u64>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::U32: break;
        default: {
          throw std::runtime_error("Bad operand type for shift amount");
        }
      }
      if (expr.op == TokenType::LShift) {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Left, right);
      }
      else {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Right, right);
      }
      break;
    }
    case TokenType::LogicalAnd:
    case TokenType::LogicalOr: {
      if (state.empty() || state.top().first == State::Read) {
        // we create a "fake local" in order to do this
        auto c_idx = emitter.c_counter++;
        std::string name = "$logop" + std::to_string(c_idx);
        variables.Set(name, calyx::CVar{
                c_idx, calyx::CVar::Location::Either, sizeof(i32)
        });
        c_types.Set(name, CType::MakeBool());
        emitter.Emit<calyx::AllocateLocal>(c_idx, sizeof(i32));

        // now basically add an if statement
        auto rhs_block = emitter.MakeBlock();
        auto true_block = emitter.MakeBlock();
        auto false_block = emitter.MakeBlock();
        auto post_block = emitter.MakeBlock();

        if (expr.op == TokenType::LogicalAnd) {
          state.push({State::ConditionalBranch, {.true_block = rhs_block, .false_block = false_block}});
        }
        else {
          state.push({State::ConditionalBranch, {.true_block = true_block, .false_block = rhs_block}});
        }
        expr.left->Visit(*this);
        state.pop();

        emitter.SelectBlock(rhs_block);
        // also check right hand side

        state.push({State::ConditionalBranch, {.true_block = true_block, .false_block = false_block}});
        expr.right->Visit(*this);
        state.pop();

        // emitter should have emitted branches to blocks
        {
          emitter.SelectBlock(true_block);
          // store result to temp cvar
          auto imm = emitter.EmitExpr<calyx::Imm<i32>>({ calyx::Var::Type::I32 }, 1);
          emitter.EmitExpr<calyx::StoreLocal<i32>>({ calyx::Var::Type::I32 }, c_idx, imm);
          emitter.Emit<calyx::UnconditionalBranch>(post_block);
        }

        {
          emitter.SelectBlock(false_block);
          // store result to temp cvar
          auto imm = emitter.EmitExpr<calyx::Imm<i32>>({ calyx::Var::Type::I32 }, 0);
          emitter.EmitExpr<calyx::StoreLocal<i32>>({ calyx::Var::Type::I32 }, c_idx, imm);
          emitter.Emit<calyx::UnconditionalBranch>(post_block);
        }

        {
          emitter.SelectBlock(post_block);
          cotyl::Assert(emitter.program.blocks[post_block].empty());
          // load result from temp cvar
          current = emitter.EmitExpr<calyx::LoadLocal<i32>>({ calyx::Var::Type::I32 }, c_idx);
        }
      }
      else if (state.top().first == State::ConditionalBranch) {
        auto rhs_block = emitter.MakeBlock();
        auto true_block = state.top().second.true_block;
        auto false_block = state.top().second.false_block;

        if (expr.op == TokenType::LogicalAnd) {
          // if lhs is false, jump to false block, otherwise jump to false block
          state.push({State::ConditionalBranch, {.true_block = rhs_block, .false_block = false_block}});
        }
        else {
          // if lhs is false, jump to rhs block, otherwise jump to true block
          state.push({State::ConditionalBranch, {.true_block = true_block, .false_block = rhs_block}});
        }
        expr.left->Visit(*this);
        state.pop();

        emitter.SelectBlock(rhs_block);
        // now we just need to check the condition like "normal"
        expr.right->Visit(*this);
      }
      else {
        // this cannot happen
      }

      // no need to check for conditional branches anymore
      return;
    }
    default:
      throw std::runtime_error("Bad binop");
  }

  if (conditional_branch) {
    auto type = emitter.vars[current].type;
    auto false_block = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(type, false_block, current, calyx::CmpType::Eq, 0);
    emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
  }
}

void ASTWalker::Visit(Ternary& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  
  if (state.empty() || state.top().first == State::Read) {
    // we create a "fake local" in order to do this
    auto c_idx = emitter.c_counter++;
    std::string name = "$tern" + std::to_string(c_idx);
    const auto& type = expr.GetType();
    u64 size = type->Sizeof();
    variables.Set(name, calyx::CVar{
            c_idx, calyx::CVar::Location::Either, size
    });
    c_types.Set(name, type);
    emitter.Emit<calyx::AllocateLocal>(c_idx, size);

    // now basically add an if statement
    auto true_block = emitter.MakeBlock();
    auto false_block = emitter.MakeBlock();
    calyx::var_index_t post_block = emitter.MakeBlock();

    state.push({State::ConditionalBranch, {.true_block = true_block, .false_block = false_block}});
    expr.cond->Visit(*this);
    state.pop();

    // emitter should have emitted branches to blocks
    {
      emitter.SelectBlock(true_block);
      expr._true->Visit(*this);
      // store result to temp cvar
      auto store_visitor = detail::EmitterTypeVisitor<detail::StoreLocalEmitter>(
              *this, { c_idx, current }
      );
      type->Visit(store_visitor);
      emitter.Emit<calyx::UnconditionalBranch>(post_block);
    }

    {
      emitter.SelectBlock(false_block);
      expr._false->Visit(*this);
      // store result to temp cvar
      auto store_visitor = detail::EmitterTypeVisitor<detail::StoreLocalEmitter>(
              *this, { c_idx, current }
      );
      type->Visit(store_visitor);
      emitter.Emit<calyx::UnconditionalBranch>(post_block);
    }

    {
      emitter.SelectBlock(post_block);
      cotyl::Assert(emitter.program.blocks[post_block].empty());
      // load result from temp cvar
      auto load_visitor = detail::EmitterTypeVisitor<detail::LoadLocalEmitter>(
              *this, { c_idx }
      );
      type->Visit(load_visitor);
    }
  }
  else {
    // for stuff like: if (var ? cond : cond)
    auto tern_true_block = emitter.MakeBlock();
    auto tern_false_block = emitter.MakeBlock();

    state.push({State::ConditionalBranch, {.true_block=tern_true_block, .false_block=tern_false_block}});
    expr.cond->Visit(*this);
    state.pop();

    // branch to false block should have been emitted
    emitter.Emit<calyx::UnconditionalBranch>(tern_true_block);
    emitter.SelectBlock(tern_true_block);

    // no need to push new state now, we have to do the original branching
    expr._true->Visit(*this);
    // branching should have been handled by conditional

    emitter.SelectBlock(tern_false_block);
    expr._false->Visit(*this);
    // again, branching should have been handled by conditional
  }
}

void ASTWalker::Visit(Assignment& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read);
  
  state.push({State::Read, {}});
  expr.right->Visit(*this);
  state.pop();

  // current now holds the expression id that we want to assign with
  auto right = current;
  calyx::BinopType op;
  switch (expr.op) {
    case TokenType::IMul: op = calyx::BinopType::Mul; break;
    case TokenType::IDiv: op = calyx::BinopType::Div; break;
    case TokenType::IMod: op = calyx::BinopType::Mod; break;
    case TokenType::IPlus: op = calyx::BinopType::Add; break;
    case TokenType::IMinus: op = calyx::BinopType::Sub; break;
    case TokenType::ILShift:
    case TokenType::IRShift: {
      // same as binop LShift / RShift
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      auto left = current;
      cotyl::Assert(!cotyl::Is(emitter.vars[left].type).AnyOf<calyx::Var::Type::Float, calyx::Var::Type::Double, calyx::Var::Type::Pointer, calyx::Var::Type::Struct>());
      cotyl::Assert(!cotyl::Is(emitter.vars[right].type).AnyOf<calyx::Var::Type::Float, calyx::Var::Type::Double>());
      switch (emitter.vars[right].type) {
        case calyx::Var::Type::I32: {
          right = emitter.EmitExpr<calyx::Cast<u32, i32>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::I64: {
          right = emitter.EmitExpr<calyx::Cast<u32, i64>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::U64: {
          right = emitter.EmitExpr<calyx::Cast<u32, u64>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::U32: break;
        default: {
          throw std::runtime_error("Bad operand type for shift amount");
        }
      }
      if (expr.op == TokenType::ILShift) {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Left, right);
      }
      else {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Right, right);
      }

      // do assignment
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();

      // todo: conditional branch
      return;
    }
    case TokenType::IAnd: op = calyx::BinopType::BinAnd; break;
    case TokenType::IOr: op = calyx::BinopType::BinOr; break;
    case TokenType::IXor: op = calyx::BinopType::BinXor; break;
    case TokenType::Assign: {
      state.push({State::Assign, {.var = right}});
      expr.left->Visit(*this);
      state.pop();

      // todo: conditional branch
      return;
    }
    default: {
      throw std::runtime_error("Invalid assignment statement");
    }
  }

  // visit left for binop assignment
  state.push({State::Read, {}});
  expr.left->Visit(*this);
  state.pop();

  auto left = current;

  // emit binop
  if (emitter.vars[left].type == calyx::Var::Type::Pointer) {
    auto var = emitter.vars[left];
    switch (op) {
      case calyx::BinopType::Add:
        EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[right].type, var.stride, left, calyx::PtrAddType::Add, var.stride, right);
        break;
      case calyx::BinopType::Sub:
        EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[right].type, var.stride, left, calyx::PtrAddType::Sub, var.stride, right);
        break;
      default:
        throw std::runtime_error("Bad pointer binop in pointer assignment");
    }
  }
  else {
    BinopHelper(left, op, right);
  }

  auto result = current;

  // do assignment
  state.push({State::Assign, {.var = current}});
  expr.left->Visit(*this);
  state.pop();

  // set current to the result variable
  current = result;
  // todo: conditional branch
}

void ASTWalker::Visit(Empty& stat) {

}

void ASTWalker::Visit(If& stat) {
  auto true_block = emitter.MakeBlock();
  auto false_block = emitter.MakeBlock();
  calyx::var_index_t post_block;
  if (stat._else) {
    post_block = emitter.MakeBlock();
  }
  else {
    post_block = false_block;
  }

  state.push({State::ConditionalBranch, {.true_block = true_block, .false_block = false_block}});
  stat.cond->Visit(*this);
  state.pop();

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

  emitter.SelectBlock(post_block);
  cotyl::Assert(emitter.program.blocks[post_block].empty());
}

void ASTWalker::Visit(While& stat) {
  
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

void ASTWalker::Visit(DoWhile& stat) {
  
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

void ASTWalker::Visit(For& stat) {
  
  auto init_block = emitter.MakeBlock();
  auto cond_block = emitter.MakeBlock();
  auto update_block = emitter.MakeBlock();
  auto loop_block = emitter.MakeBlock();
  auto post_block = emitter.MakeBlock();

  // new scope for declarations in for loop
  variables.NewLayer();

  // always go to initialization
  emitter.Emit<calyx::UnconditionalBranch>(init_block);
  emitter.SelectBlock(init_block);
  for (auto& decl : stat.decls) {
    decl->Visit(*this);
  }
  for (auto& init : stat.inits) {
    init->Visit(*this);
  }

  // go to condition
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);
  emitter.SelectBlock(cond_block);

  state.push({State::ConditionalBranch, {.true_block = loop_block, .false_block = post_block}});
  stat.cond->Visit(*this);
  state.pop();

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

  // pop variables layer
  // todo: dealloc?
//  for (auto var = variables.Top().rbegin(); var != variables.Top().rend(); var++) {
//    emitter.Emit<calyx::DeallocateCVar>(var->second.idx, var->second.size);
//  }
  variables.PopLayer();

  // go to block after loop
  emitter.SelectBlock(post_block);
}

void ASTWalker::Visit(Label& stat) {
  if (!emitter.program.local_labels.contains(stat.name)) {
    auto block = emitter.MakeBlock();
    emitter.program.local_labels.emplace(stat.name, block);
    emitter.Emit<calyx::UnconditionalBranch>(block);
    emitter.SelectBlock(block);
  }
  else {
    auto block = emitter.program.local_labels.at(stat.name);
    emitter.Emit<calyx::UnconditionalBranch>(block);
    emitter.SelectBlock(block);
  }
  stat.stat->Visit(*this);
}

void ASTWalker::Visit(Switch& stat) {
  state.push({State::Read, {}});
  stat.expr->Visit(*this);
  state.pop();

  auto right = current;
  switch (emitter.vars[right].type) {
    case calyx::Var::Type::I32: {
      right = emitter.EmitExpr<calyx::Cast<i64, i32>>({ calyx::Var::Type::U32 }, right);
      break;
    }
    case calyx::Var::Type::U32: {
      right = emitter.EmitExpr<calyx::Cast<i64, u32>>({ calyx::Var::Type::U32 }, right);
      break;
    }
    case calyx::Var::Type::U64: {
      right = emitter.EmitExpr<calyx::Cast<i64, u64>>({ calyx::Var::Type::U32 }, right);
      break;
    }
    case calyx::Var::Type::I64: break;
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

void ASTWalker::Visit(Case& stat) {
  cotyl::Assert(!select_stack.empty(), "Invalid case statement");
  auto* select = select_stack.top();
  cotyl::Assert(!select->table.contains(stat.expr), "Duplicate case statement");

  auto block = emitter.MakeBlock();
  // check for fallthrough and don't emit branch if there is no fallthrough
  emitter.Emit<calyx::UnconditionalBranch>(block);

  select->table.emplace(stat.expr, block);

  emitter.SelectBlock(block);
  stat.stat->Visit(*this);
}

void ASTWalker::Visit(Default& stat) {
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

void ASTWalker::Visit(Goto& stat) {
  if (!emitter.program.local_labels.contains(stat.label)) {
    auto block = emitter.MakeBlock();
    emitter.program.local_labels.emplace(stat.label, block);
    emitter.Emit<calyx::UnconditionalBranch>(block);
  }
  else {
    emitter.Emit<calyx::UnconditionalBranch>(emitter.program.local_labels.at(stat.label));
  }
}

void ASTWalker::Visit(Return& stat) {
  if (stat.expr) {
    state.push({State::Read, {}});
    stat.expr->Visit(*this);
    auto visitor = detail::EmitterTypeVisitor<detail::ReturnEmitter>(*this, { current });
    function->signature->contained->Visit(visitor);
    state.pop();
  }
  else {
    emitter.Emit<calyx::Return<i32>>(0);
  }
}

void ASTWalker::Visit(Break& stat) {
  cotyl::Assert(!break_stack.empty());
  emitter.Emit<calyx::UnconditionalBranch>(break_stack.top());
}

void ASTWalker::Visit(Continue& stat) {
  cotyl::Assert(!continue_stack.empty());
  emitter.Emit<calyx::UnconditionalBranch>(continue_stack.top());
}

void ASTWalker::Visit(Compound& stat) {
  variables.NewLayer();
  for (const auto& node : stat.stats) {
    node->Visit(*this);
  }
  for (auto var = variables.Top().rbegin(); var != variables.Top().rend(); var++) {
    emitter.Emit<calyx::DeallocateLocal>(var->second.idx, var->second.size);
  }
  variables.PopLayer();
}

}
