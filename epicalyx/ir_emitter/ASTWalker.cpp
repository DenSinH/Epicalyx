#include "ASTWalker.h"
#include "Emitter.h"

#include "calyx/Calyx.h"
#include "calyx/backend/interpreter/Interpreter.h"
#include "types/EpiCType.h"
#include "ast/Declaration.h"
#include "ast/Statement.h"
#include "ast/Expression.h"

#include "Is.h"
#include "Cast.h"
#include "Exceptions.h"
#include "CustomAssert.h"

#include "Helpers.inl"


namespace epi {

namespace detail {
  
struct CalyxLocalTypeVisitor : TypeVisitor {
  calyx::Local::Type local_type;

  void Visit(const VoidType& type) final { throw std::runtime_error("Incomplete type"); }
  void Visit(const ValueType<i8>& type) final { local_type = calyx::Local::Type::I8; }
  void Visit(const ValueType<u8>& type) final { local_type = calyx::Local::Type::U8; }
  void Visit(const ValueType<i16>& type) final { local_type = calyx::Local::Type::I16; }
  void Visit(const ValueType<u16>& type) final { local_type = calyx::Local::Type::U16; }
  void Visit(const ValueType<i32>& type) final { local_type = calyx::Local::Type::I32; }
  void Visit(const ValueType<u32>& type) final { local_type = calyx::Local::Type::U32; }
  void Visit(const ValueType<i64>& type) final { local_type = calyx::Local::Type::I64; }
  void Visit(const ValueType<u64>& type) final { local_type = calyx::Local::Type::U64; }
  void Visit(const ValueType<float>& type) final { local_type = calyx::Local::Type::Float; }
  void Visit(const ValueType<double>& type) final { local_type = calyx::Local::Type::Double; }
  void Visit(const PointerType& type) final { local_type = calyx::Local::Type::Pointer; }
  void Visit(const ArrayType& type) final { local_type = calyx::Local::Type::Pointer; }
  void Visit(const FunctionType& type) final { local_type = calyx::Local::Type::Pointer; }
  void Visit(const StructType& type) final { local_type = calyx::Local::Type::Struct; }
  void Visit(const UnionType& type) final { local_type = calyx::Local::Type::Struct; }
};

}

calyx::Local::Type ASTWalker::GetCalyxType(const pType<const CType>& type) {
  detail::CalyxLocalTypeVisitor visitor{};
  type->Visit(visitor);
  return visitor.local_type;
}


void ASTWalker::Visit(epi::ast::DeclarationNode& decl) {
  if (locals.Depth() == 1) {
    // global symbols
    AddGlobal(decl.name, decl.type);

    auto global_init_visitor = detail::GlobalInitializerVisitor();
    decl.type->Visit(global_init_visitor);
    calyx::global_t& global = emitter.program.globals[decl.name] = global_init_visitor.result;

    if (decl.value.has_value()) {
      if (std::holds_alternative<pExpr>(decl.value.value())) {
        auto& expr = std::get<pExpr>(decl.value.value());

        calyx::Function initializer{"$init" + decl.name};
        emitter.SetFunction(initializer);

        state.push({State::Read, {}});
        expr->Visit(*this);
        state.pop();

        auto global_block_return_visitor = detail::EmitterTypeVisitor<detail::ReturnEmitter>(*this, { current });
        decl.type->Visit(global_block_return_visitor);

        calyx::Interpreter interpreter = {emitter.program};
        interpreter.InterpretGlobalInitializer(global, std::move(initializer));
      }
      else {
        // todo: handle initializer list
        throw cotyl::UnimplementedException("global initializer list declaration");
      }
    }
  }
  else {
    auto c_idx = AddLocal(decl.name, decl.type);

    if (decl.value.has_value()) {
      if (std::holds_alternative<pExpr>(decl.value.value())) {
        state.push({State::Read, {}});
        std::get<pExpr>(decl.value.value())->Visit(*this);
        state.pop();
        // current now holds the expression id that we want to assign with
        state.push({State::Assign, {.var = current}});
        IdentifierNode(decl.name).Visit(*this);
        state.pop();
      }
      else {
        // todo: handle initializer list
        throw cotyl::UnimplementedException("initializer list declaration");
      }
    }
  }
}

void ASTWalker::Visit(FunctionDefinitionNode& decl) {
  NewFunction(decl.symbol, decl.signature);

  // same as normal compound statement besides arguments
  locals.NewLayer();
  for (int i = 0; i < decl.signature->arg_types.size(); i++) {
    // turn arguments into locals
    auto& arg = decl.signature->arg_types[i];
    auto c_idx = AddLocal(arg.name, arg.type);
    auto visitor = detail::ArgumentTypeVisitor(i, false);
    arg.type->Visit(visitor);

    emitter.Emit<calyx::ArgMakeLocal>(visitor.result, c_idx);
  }

  // locals layer
  locals.NewLayer();

  function = &decl;
  for (const auto& node : decl.body->stats) {
    node->Visit(*this);
  }
  emitter.Emit<calyx::Return<void>>(0);
  // locals layer
  locals.PopLayer();

  // arguments layer
  locals.PopLayer();

  EndFunction();
}

void ASTWalker::Visit(IdentifierNode& decl) {
  // after the AST, the only identifiers left are C locals
  if (state.top().first == State::Empty) {
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
    auto fblock = state.top().second.false_block;
    auto tblock = state.top().second.true_block;
    EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, tblock, fblock, current, calyx::CmpType::Ne, 0);
    return;
  }

  auto type = GetSymbolType(decl.name);
  if (locals.Has(decl.name)) {
    // local variable
    const auto& cvar = *locals.Get(decl.name).loc;
    switch (state.top().first) {
      case State::Read: {
        if (type->IsArray()) {
          current = emitter.EmitExpr<calyx::LoadLocalAddr>({Emitter::Var::Type::Pointer, type->Deref()->Sizeof() }, cvar.idx);
        }
        else {
          auto visitor = detail::EmitterTypeVisitor<detail::LoadLocalEmitter>(*this, {cvar.idx});
          type->Visit(visitor);
        }
        break;
      }
      case State::Assign: {
        const auto stored = state.top().second.var;
        auto visitor = detail::EmitterTypeVisitor<detail::StoreLocalEmitter>(
                *this, { cvar.idx, stored }
        );
        type->Visit(visitor);
        current = stored;
        break;
      }
      case State::Address: {
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
          current = emitter.EmitExpr<calyx::LoadGlobalAddr>({Emitter::Var::Type::Pointer, type->Deref()->Sizeof() }, decl.name);
        }
        else if (type->IsFunction()) {
          auto visitor = detail::EmitterTypeVisitor<detail::LoadGlobalAddrEmitter>(*this, {decl.name});
          type->Visit(visitor);
        }
        else {
          auto visitor = detail::EmitterTypeVisitor<detail::LoadGlobalEmitter>(*this, {decl.name});
          type->Visit(visitor);
        }
        break;
      }
      case State::Assign: {
        const auto stored = state.top().second.var;
        auto visitor = detail::EmitterTypeVisitor<detail::StoreGlobalEmitter>(
                *this, { decl.name, stored }
        );
        type->Visit(visitor);
        current = stored;
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
void ASTWalker::ConstVisitImpl(NumericalConstantNode<T>& expr) {
  if (state.top().first == State::Empty) {
    // statement has no effect
    return;
  }
  cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  if (state.top().first == State::ConditionalBranch) {
    if (expr.value) {
      emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
    }
    else {
      emitter.Emit<calyx::UnconditionalBranch>(state.top().second.false_block);
    }
  }
  else {
    current = emitter.EmitExpr<calyx::Imm<calyx::calyx_upcast_t<T>>>({ detail::calyx_var_type_v<calyx::calyx_upcast_t<T>> }, expr.value);
  }
}

template void ASTWalker::ConstVisitImpl(NumericalConstantNode<i8>&);
template void ASTWalker::ConstVisitImpl(NumericalConstantNode<u8>&);
template void ASTWalker::ConstVisitImpl(NumericalConstantNode<i16>&);
template void ASTWalker::ConstVisitImpl(NumericalConstantNode<u16>&);
template void ASTWalker::ConstVisitImpl(NumericalConstantNode<i32>&);
template void ASTWalker::ConstVisitImpl(NumericalConstantNode<u32>&);
template void ASTWalker::ConstVisitImpl(NumericalConstantNode<i64>&);
template void ASTWalker::ConstVisitImpl(NumericalConstantNode<u64>&);
template void ASTWalker::ConstVisitImpl(NumericalConstantNode<float>&);
template void ASTWalker::ConstVisitImpl(NumericalConstantNode<double>&);

void ASTWalker::Visit(StringConstantNode& expr) {
  // load from rodata
  throw cotyl::UnimplementedException("string constant");
}

void ASTWalker::Visit(ArrayAccessNode& expr) {
  if (state.top().first == State::Empty) {
    // increments/decrements might happen
    expr.left->Visit(*this);
    expr.right->Visit(*this);
    return;
  }
  cotyl::Assert(cotyl::Is(state.top().first).AnyOf<State::Read, State::ConditionalBranch, State::Assign, State::Address>());

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
          emitter.vars[offs_idx].type, ptr_var.stride, ptr_idx, ptr_var.stride, offs_idx
  );
  ptr_idx = current;

  if (state.top().first == State::Read || state.top().first == State::ConditionalBranch) {
    auto visitor = detail::EmitterTypeVisitor<detail::LoadFromPointerEmitter>(*this, { ptr_idx });
    if (expr.left->GetType()->IsPointer()) {
      expr.left->GetType()->Deref()->Visit(visitor);
    }
    else {
      expr.right->GetType()->Deref()->Visit(visitor);
    }

    if (state.top().first == State::ConditionalBranch) {
      auto tblock = state.top().second.true_block;
      auto fblock = state.top().second.false_block;
      EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, tblock, fblock, current, calyx::CmpType::Ne, 0);
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

void ASTWalker::Visit(FunctionCallNode& expr) {
  state.push({State::Read, {}});
  expr.left->Visit(*this);
  state.pop();

  const auto fn = current;
  const auto* signature = cotyl::shared_ptr_cast<const FunctionType>(expr.left->GetType());
  const auto num_args = signature->arg_types.size();
  calyx::arg_list_t args{};
  calyx::arg_list_t var_args{};

  for (int i = 0; i < num_args; i++) {
    expr.args[i]->Visit(*this);
    auto cast_arg_visitor = detail::EmitterTypeVisitor<detail::CastToEmitter>(*this, { current });
    signature->arg_types[i].type->Visit(cast_arg_visitor);

    auto arg_visitor = detail::ArgumentTypeVisitor(i, false);
    signature->arg_types[i].type->Visit(arg_visitor);

    args.emplace_back(current, arg_visitor.result);
  }

  if (signature->variadic) {
    for (int i = num_args; i < expr.args.size(); i++) {
      expr.args[i]->Visit(*this);

      auto arg_visitor = detail::ArgumentTypeVisitor(i - num_args, true);
      expr.args[i]->GetType()->Visit(arg_visitor);

      var_args.emplace_back(current, arg_visitor.result);
    }
  }

  if (signature->contained->IsVoid()) {
    emitter.Emit<calyx::Call<void>>(0, fn, std::move(args), std::move(var_args));
  }
  else {
    auto visitor = detail::EmitterTypeVisitor<detail::CallEmitter>(*this, { fn, std::move(args), std::move(var_args) });
    signature->contained->Visit(visitor);

    if (state.top().first == State::ConditionalBranch) {
      auto tblock = state.top().second.true_block;
      auto fblock = state.top().second.false_block;
      EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, tblock, fblock, current, calyx::CmpType::Ne, 0);
    }
  }
}

void ASTWalker::Visit(MemberAccessNode& expr) {
  throw cotyl::UnimplementedException("member access");
}

void ASTWalker::Visit(TypeInitializerNode& expr) {
  throw cotyl::UnimplementedException("type initializer");
}

void ASTWalker::Visit(PostFixNode& expr) {
  cotyl::Assert(state.top().first == State::Empty || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  
  bool conditional_branch = state.top().first == State::ConditionalBranch;

  switch (expr.op) {
    case TokenType::Incr: {
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();
      auto read = current;
      auto type = emitter.vars[current].type;
      if (type == Emitter::Var::Type::Pointer) {
        auto var = emitter.vars[read];
        current = emitter.EmitExpr<calyx::AddToPointerImm>(var, read, var.stride, 1);
      }
      else if (type == Emitter::Var::Type::Struct) {
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
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();
      auto read = current;
      auto type = emitter.vars[current].type;
      if (type == Emitter::Var::Type::Pointer) {
        auto var = emitter.vars[read];
        current = emitter.EmitExpr<calyx::AddToPointerImm>(var, read, var.stride, -1);
      }
      else if (type == Emitter::Var::Type::Struct) {
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
    auto tblock = state.top().second.true_block;
    auto fblock = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, tblock, fblock, current, calyx::CmpType::Ne, 0);
  }
}

void ASTWalker::Visit(UnopNode& expr) {
  const bool conditional_branch = state.top().first == State::ConditionalBranch;

  switch (expr.op) {
    case TokenType::Minus: {
      if (state.top().first == State::Empty) {
        expr.left->Visit(*this);
        return;
      }
      cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      // no need to push a new state
      expr.left->Visit(*this);
      if (!conditional_branch) {
        EmitArithExpr<calyx::Unop>(emitter.vars[current].type, calyx::UnopType::Neg, current);
      }
      // return, no need to check for conditional branches, is handled in visiting the expr
      return;
    }
    case TokenType::Plus: {
      cotyl::Assert(state.top().first == State::Empty || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      // no need to push a new state
      // does nothing
      // return, no need to check for conditional branches, is handled in visiting the expr
      expr.left->Visit(*this);
      return;
    }
    case TokenType::Tilde: {
      if (state.top().first == State::Empty) {
        expr.left->Visit(*this);
        return;
      }
      cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      // no need to push a new state
      expr.left->Visit(*this);
      EmitArithExpr<calyx::Unop>(emitter.vars[current].type, calyx::UnopType::BinNot, current);
      // break, need to check for conditional branches
      break;
    }
    case TokenType::Exclamation: {
      if (state.top().first == State::Empty) {
        expr.left->Visit(*this);
        return;
      }
      cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      if (conditional_branch) {
        auto tblock = state.top().second.true_block;
        auto fblock = state.top().second.false_block;
        // branch to false if current != 0 (if current == 0, then !current is true)
        EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, fblock, tblock, current, calyx::CmpType::Ne, 0);
      }
      else {
        EmitCompare<calyx::CompareImm>(emitter.vars[current].type, current, calyx::CmpType::Eq, 0);
      }

      // return, no need to check for conditional branches, already handled
      return;
    }
    case TokenType::Incr: {
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      auto type = emitter.vars[current].type;
      calyx::var_index_t stored;
      if (type == Emitter::Var::Type::Pointer) {
        auto var = emitter.vars[current];
        current = emitter.EmitExpr<calyx::AddToPointerImm>(var, current, var.stride, 1);
      }
      else if (type == Emitter::Var::Type::Struct) {
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
      // read value
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      auto type = emitter.vars[current].type;
      calyx::var_index_t stored;

      // subtract 1
      if (type == Emitter::Var::Type::Pointer) {
        auto var = emitter.vars[current];
        current = emitter.EmitExpr<calyx::AddToPointerImm>(var, current, var.stride, -1);
      }
      else if (type == Emitter::Var::Type::Struct) {
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
      if (state.top().first == State::Empty) {
        expr.left->Visit(*this);
        return;
      }
      cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
      state.push({ State::Address, {} });
      expr.left->Visit(*this);
      state.pop();
      break;
    }
    case TokenType::Asterisk: {
      if (state.top().first == State::Empty) {
        expr.left->Visit(*this);
        return;
      }
      else if (state.top().first == State::Assign) {
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
      else if (state.top().first == State::Address) {
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
    auto tblock = state.top().second.true_block;
    auto fblock = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, tblock, fblock, current, calyx::CmpType::Ne, 0);
  }
}

void ASTWalker::Visit(CastNode& expr) {
  if (state.top().first == State::Empty) {
    expr.expr->Visit(*this);
    return;
  }
  cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  // no need to push a new state
  expr.expr->Visit(*this);

  // only need to emit an actual cast if we are not checking for 0
  if (state.top().first != State::ConditionalBranch) {
    auto visitor = detail::EmitterTypeVisitor<detail::CastToEmitter>(*this, { current });
    expr.type->Visit(visitor);
  }
}

void ASTWalker::Visit(BinopNode& expr) {
  if (state.top().first == State::Empty) {
    // increments / decrements might happen on either side
    expr.left->Visit(*this);
    expr.right->Visit(*this);
    return;
  }
  cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  
  // only need to push a new state for conditional branches
  const bool conditional_branch = state.top().first == State::ConditionalBranch;
  calyx::var_index_t left, right;

  if (expr.op != TokenType::LogicalAnd && expr.op != TokenType::LogicalOr) {
    if (conditional_branch) {
      state.push({State::Read, {}});
    }
    expr.left->Visit(*this);
    left = current;
    expr.right->Visit(*this);
    right = current;
    if (conditional_branch) {
      state.pop();
    }
  }

  switch (expr.op) {
    case TokenType::Asterisk: BinopHelper(left, calyx::BinopType::Mul, right); break;
    case TokenType::Div: BinopHelper(left, calyx::BinopType::Div, right); break;
    case TokenType::Mod: BinopHelper(left, calyx::BinopType::Mod, right); break;
    case TokenType::Ampersand: BinopHelper(left, calyx::BinopType::BinAnd, right); break;
    case TokenType::BinOr: BinopHelper(left, calyx::BinopType::BinOr, right); break;
    case TokenType::BinXor: BinopHelper(left, calyx::BinopType::BinXor, right); break;
    case TokenType::Plus: {
      if (emitter.vars[left].type == Emitter::Var::Type::Pointer) {
        auto var = emitter.vars[left];
        EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[right].type, var.stride, left, var.stride, right);
      }
      else if (emitter.vars[right].type == Emitter::Var::Type::Pointer) {
        auto var = emitter.vars[right];
        EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[left].type, var.stride, right, var.stride, left);
      }
      else {
        BinopHelper(left, calyx::BinopType::Add, right);
      }
      break;
    }
    case TokenType::Minus: {
      if (emitter.vars[left].type == Emitter::Var::Type::Pointer) {
        auto var = emitter.vars[left];
        if (emitter.vars[right].type == Emitter::Var::Type::Pointer) {
          throw cotyl::UnimplementedException("pointer diff");
        }
        else {
          EmitIntegralExpr<calyx::Unop>(emitter.vars[right].type, calyx::UnopType::Neg, right);
          EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[right].type, var.stride, left, var.stride, current);
        }
      }
      else if (emitter.vars[right].type == Emitter::Var::Type::Pointer) {
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
      switch (expr.op) {
        case TokenType::Less:
          cmp_type = calyx::CmpType::Lt;
          break;
        case TokenType::LessEqual:
          cmp_type = calyx::CmpType::Le;
          break;
        case TokenType::GreaterEqual:
          cmp_type = calyx::CmpType::Ge;
          break;
        case TokenType::Greater:
          cmp_type = calyx::CmpType::Gt;
          break;
        case TokenType::Equal:
          cmp_type = calyx::CmpType::Eq;
          break;
        case TokenType::NotEqual:
          cmp_type = calyx::CmpType::Ne;
          break;
        default:
          throw std::runtime_error("Unreachable");
      }

      if (conditional_branch) {
        auto tblock = state.top().second.true_block;
        auto fblock = state.top().second.false_block;
        EmitBranch<calyx::BranchCompare>(casted.var.type, tblock, fblock, casted.left, cmp_type, casted.right);
      }
      else {
        EmitCompare<calyx::Compare>(casted.var.type, casted.left, cmp_type, casted.right);
      }
      // no need to check for conditional branching after this
      return;
    }
    case TokenType::LShift:
    case TokenType::RShift: {
      cotyl::Assert(!cotyl::Is(emitter.vars[left].type).AnyOf<Emitter::Var::Type::Float, Emitter::Var::Type::Double, Emitter::Var::Type::Pointer, Emitter::Var::Type::Struct>());
      cotyl::Assert(!cotyl::Is(emitter.vars[right].type).AnyOf<Emitter::Var::Type::Float, Emitter::Var::Type::Double, Emitter::Var::Type::Pointer, Emitter::Var::Type::Struct>());
      switch (emitter.vars[right].type) {
        case Emitter::Var::Type::I32: {
          right = emitter.EmitExpr<calyx::Cast<u32, i32>>({ Emitter::Var::Type::U32 }, right);
          break;
        }
        case Emitter::Var::Type::I64: {
          right = emitter.EmitExpr<calyx::Cast<u32, i64>>({ Emitter::Var::Type::U32 }, right);
          break;
        }
        case Emitter::Var::Type::U64: {
          right = emitter.EmitExpr<calyx::Cast<u32, u64>>({ Emitter::Var::Type::U32 }, right);
          break;
        }
        case Emitter::Var::Type::U32: break;
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
      if (state.top().first == State::Read) {
        // we create a "fake local" in order to do this
        std::string name = "$logop" + std::to_string(emitter.c_counter);
        auto c_idx = AddLocal(name, CType::MakeBool());
        
        // now basically add an if statement
        auto rhs_block = emitter.MakeBlock();
        auto true_block = emitter.MakeBlock();
        auto false_block = emitter.MakeBlock();

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

        // make post block here to improve optimization ancestry finding
        auto post_block = emitter.MakeBlock();
        // emitter should have emitted branches to blocks
        {
          emitter.SelectBlock(true_block);
          // store result to temp cvar
          auto imm = emitter.EmitExpr<calyx::Imm<i32>>({ Emitter::Var::Type::I32 }, 1);
          emitter.Emit<calyx::StoreLocal<i32>>(c_idx, imm);
          current = imm;
          emitter.Emit<calyx::UnconditionalBranch>(post_block);
        }

        {
          emitter.SelectBlock(false_block);
          // store result to temp cvar
          auto imm = emitter.EmitExpr<calyx::Imm<i32>>({ Emitter::Var::Type::I32 }, 0);
          emitter.Emit<calyx::StoreLocal<i32>>(c_idx, imm);
          current = imm;
          emitter.Emit<calyx::UnconditionalBranch>(post_block);
        }

        {
          emitter.SelectBlock(post_block);
          cotyl::Assert(emitter.current_function->blocks.at(post_block).empty());
          // load result from temp cvar
          current = emitter.EmitExpr<calyx::LoadLocal<i32>>({ Emitter::Var::Type::I32 }, c_idx);
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
    auto tblock = state.top().second.true_block;
    auto fblock = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(type, tblock, fblock, current, calyx::CmpType::Ne, 0);
  }
}

void ASTWalker::Visit(TernaryNode& expr) {
  cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  
  if (state.top().first == State::Read) {
    // we create a "fake local" in order to do this
    std::string name = "$tern" + std::to_string(emitter.c_counter);
    auto type = expr.GetType();
    auto c_idx = AddLocal(name, type);

    // now basically add an if statement
    auto true_block = emitter.MakeBlock();
    auto false_block = emitter.MakeBlock();

    state.push({State::ConditionalBranch, {.true_block = true_block, .false_block = false_block}});
    expr.cond->Visit(*this);
    state.pop();

    // create post block here to improve optimization ancestry finding
    auto post_block = emitter.MakeBlock();
    // emitter should have emitted branches to blocks
    {
      emitter.SelectBlock(true_block);
      expr._true->Visit(*this);
      // store result to temp cvar
      const auto stored = current;
      auto store_visitor = detail::EmitterTypeVisitor<detail::StoreLocalEmitter>(
              *this, { c_idx, stored }
      );
      type->Visit(store_visitor);
      current = stored;
      emitter.Emit<calyx::UnconditionalBranch>(post_block);
    }

    {
      emitter.SelectBlock(false_block);
      expr._false->Visit(*this);
      // store result to temp cvar
      const auto stored = current;
      auto store_visitor = detail::EmitterTypeVisitor<detail::StoreLocalEmitter>(
              *this, { c_idx, stored }
      );
      type->Visit(store_visitor);
      current = stored;
      emitter.Emit<calyx::UnconditionalBranch>(post_block);
    }

    {
      emitter.SelectBlock(post_block);
      cotyl::Assert(emitter.current_function->blocks.at(post_block).empty());
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

void ASTWalker::Visit(AssignmentNode& expr) {
  cotyl::Assert(state.top().first == State::Empty || state.top().first == State::Read);
  
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
      cotyl::Assert(!cotyl::Is(emitter.vars[left].type).AnyOf<Emitter::Var::Type::Float, Emitter::Var::Type::Double, Emitter::Var::Type::Pointer, Emitter::Var::Type::Struct>());
      cotyl::Assert(!cotyl::Is(emitter.vars[right].type).AnyOf<Emitter::Var::Type::Float, Emitter::Var::Type::Double>());
      switch (emitter.vars[right].type) {
        case Emitter::Var::Type::I32: {
          right = emitter.EmitExpr<calyx::Cast<u32, i32>>({ Emitter::Var::Type::U32 }, right);
          break;
        }
        case Emitter::Var::Type::I64: {
          right = emitter.EmitExpr<calyx::Cast<u32, i64>>({ Emitter::Var::Type::U32 }, right);
          break;
        }
        case Emitter::Var::Type::U64: {
          right = emitter.EmitExpr<calyx::Cast<u32, u64>>({ Emitter::Var::Type::U32 }, right);
          break;
        }
        case Emitter::Var::Type::U32: break;
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
  if (emitter.vars[left].type == Emitter::Var::Type::Pointer) {
    auto var = emitter.vars[left];
    switch (op) {
      case calyx::BinopType::Add:
        EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[right].type, var.stride, left, var.stride, right);
        break;
      case calyx::BinopType::Sub: {
        EmitIntegralExpr<calyx::Unop>(emitter.vars[right].type, calyx::UnopType::Neg, right);
        EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[right].type, var.stride, left, var.stride, current);
        break;
      }
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

void ASTWalker::Visit(EmptyNode& stat) {

}

void ASTWalker::Visit(IfNode& stat) {
  auto true_block = emitter.MakeBlock();
  auto false_block = emitter.MakeBlock();
  calyx::block_label_t post_block;

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
  cotyl::Assert(emitter.current_function->blocks.at(post_block).empty());
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
    decl->Visit(*this);
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
  cotyl::Assert(!select->table.contains(stat.expr), "Duplicate case statement");

  auto block = emitter.MakeBlock();
  // check for fallthrough and don't emit branch if there is no fallthrough
  emitter.Emit<calyx::UnconditionalBranch>(block);

  select->table.emplace(stat.expr, block);

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
    function->signature->contained->Visit(visitor);
    state.pop();
  }
  else {
    emitter.Emit<calyx::Return<void>>(0);
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
