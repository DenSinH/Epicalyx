#include "ASTWalker.h"
#include "Emitter.h"
#include "Helpers.h"

#include "tokenizer/TokenType.h"
#include "types/Types.h"
#include "calyx/Directive.h"
#include "ast/Expression.h"
#include "ast/Declaration.h"

#include "Is.h"
#include "Exceptions.h"
#include "CustomAssert.h"

#include "Helpers.inl"
#include "Helpers.Expression.inl"


namespace epi {

using namespace ast;

void ASTWalker::EmitConditionalBranchForCurrent() {
  cotyl::Assert(state.top().first == State::ConditionalBranch);
  // emit branch
  // branch to false block if 0, otherwise go to true block
  auto fblock = state.top().second.false_block;
  auto tblock = state.top().second.true_block;
  auto left = current;
  auto type = emitter.vars[left].type;
  if (type == Emitter::Var::Type::Pointer) {
    current = emitter.EmitExpr<calyx::Imm<calyx::Pointer>>({ type, emitter.vars[left].stride }, 0);
  }
  else {
    EmitArithExpr<calyx::Imm>(type, 0);
  }
  auto imm = current;
  EmitBranch<calyx::BranchCompare>(type, tblock, fblock, left, calyx::CmpType::Ne, imm);
}

void ASTWalker::BinopHelper(var_index_t left, calyx::BinopType op, var_index_t right) {
  auto casted = BinopCastHelper(left, right);
  EmitArithExpr<calyx::Binop>(casted.var.type, casted.left, op, casted.right);
}

void ASTWalker::Visit(const IdentifierNode& decl) {
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

    EmitConditionalBranchForCurrent();
    return;
  }

  auto type = GetSymbolType(decl.name);
  if (locals.Has(decl.name)) {
    // local variable
    const auto& cvar = *locals.Get(decl.name).loc;
    switch (state.top().first) {
      case State::Read: {
        if (type.holds_alternative<type::ArrayType>() && type.get<type::ArrayType>().size) {
          current = emitter.EmitExpr<calyx::LoadLocalAddr>({Emitter::Var::Type::Pointer, type.get<type::ArrayType>().Stride() }, cvar.idx);
        }
        else if (type.holds_alternative<type::StructType>() || type.holds_alternative<type::UnionType>()) {
          current = emitter.EmitExpr<calyx::LoadLocalAddr>({Emitter::Var::Type::Pointer, 0 }, cvar.idx);
        }
        else {
          auto visitor = detail::EmitterTypeVisitor<detail::LoadLocalEmitter>(
                  *this, {cvar.idx}
          );
          visitor.Visit(type);
        }
        break;
      }
      case State::Assign: {
        const auto stored = state.top().second.var;
        auto visitor = detail::EmitterTypeVisitor<detail::StoreLocalEmitter>(
                *this, { cvar.idx, stored }
        );
        visitor.Visit(type);
        current = stored;
        break;
      }
      case State::Address: {
        auto visitor = detail::EmitterTypeVisitor<detail::LoadLocalAddrEmitter>(
                *this, {cvar.idx}
        );
        visitor.Visit(type);
        break;
      }
      default: {
        throw EmitterError("Bad declaration state");
      }
    }
  }
  else {
    // global symbol
    switch (state.top().first) {
      case State::Read: {
        if (type.holds_alternative<type::ArrayType>()) {
          current = emitter.EmitExpr<calyx::LoadGlobalAddr>({Emitter::Var::Type::Pointer, type->Deref()->Sizeof() }, cotyl::CString{decl.name});
        }
        else if (type.holds_alternative<type::StructType>() || type.holds_alternative<type::UnionType>()) {
          current = emitter.EmitExpr<calyx::LoadGlobalAddr>({Emitter::Var::Type::Pointer, 0 }, cotyl::CString{decl.name});
        }
        else if (type.holds_alternative<type::FunctionType>()) {
          auto visitor = detail::EmitterTypeVisitor<detail::LoadGlobalAddrEmitter>(*this, {cotyl::CString{decl.name}});
          visitor.Visit(type);
        }
        else {
          auto visitor = detail::EmitterTypeVisitor<detail::LoadGlobalEmitter>(*this, {cotyl::CString{decl.name}});
          visitor.Visit(type);
        }
        break;
      }
      case State::Assign: {
        const auto stored = state.top().second.var;
        auto visitor = detail::EmitterTypeVisitor<detail::StoreGlobalEmitter>(
                *this, { cotyl::CString{decl.name}, stored }
        );
        visitor.Visit(type);
        current = stored;
        break;
      }
      case State::Address: {
        auto visitor = detail::EmitterTypeVisitor<detail::LoadGlobalAddrEmitter>(*this, {cotyl::CString{decl.name}});
        visitor.Visit(type);
        break;
      }
      default: {
        throw EmitterError("Bad declaration state");
      }
    }
  }
}

template<typename T>
void ASTWalker::ConstVisitImpl(const NumericalConstantNode<T>& expr) {
  if (state.top().first == State::Empty) {
    // statement has no effect
    return;
  }
  cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  if (state.top().first == State::ConditionalBranch) {
    if (expr.value) emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
    else            emitter.Emit<calyx::UnconditionalBranch>(state.top().second.false_block);
  }
  else {
    current = emitter.EmitExpr<calyx::Imm<calyx::calyx_upcast_t<T>>>({ detail::calyx_var_type_v<calyx::calyx_upcast_t<T>> }, expr.value);
  }
}

template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<i8>&);
template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<u8>&);
template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<i16>&);
template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<u16>&);
template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<i32>&);
template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<u32>&);
template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<i64>&);
template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<u64>&);
template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<float>&);
template void ASTWalker::ConstVisitImpl(const NumericalConstantNode<double>&);

void ASTWalker::Visit(const StringConstantNode& expr) {
  switch (state.top().first) {
    case State::Empty: return;  // has no effect
    case State::ConditionalBranch: {
      // always branch, pointer is not null
      emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
      return;
    }
    case State::Read:
    case State::Address: {
      // both give the address of the local
      cotyl::CString global_name{".str" + std::to_string(emitter.program.globals.size())};
      
      // null terminator
      calyx::AggregateData data{expr.value.size() + 1, 1};
      std::memcpy(data.data.get(), expr.value.c_str(), expr.value.size());

      emitter.program.globals.emplace(global_name, std::move(data));
      current = emitter.EmitExpr<calyx::LoadGlobalAddr>({Emitter::Var::Type::Pointer, 1 }, std::move(global_name));
      return;
    }
    default: 
      // String constant cannot be assigned
      throw cotyl::UnreachableException();
  }
}

void ASTWalker::Visit(const ArrayAccessNode& expr) {
  if (state.top().first == State::Empty) {
    // increments/decrements might happen
    expr.ptr->Visit(*this);
    expr.offs->Visit(*this);
    return;
  }
  cotyl::Assert(cotyl::Is(state.top().first).AnyOf<State::Read, State::ConditionalBranch, State::Assign, State::Address>());
  const auto& ptrtype = expr.ptr->type;
  u64 stride = 0;
  var_index_t ptr_idx, offs_idx;
  if (ptrtype.holds_alternative<type::ArrayType>()) {
    // pointer to array, load address again
    state.push({State::Address, {}});
    stride = ptrtype.get<type::ArrayType>().Stride();
  }
  else {
    cotyl::Assert(ptrtype.holds_alternative<type::PointerType>());
    // pointer to pointer, load pointer
    state.push({State::Read, {}});
    stride = ptrtype.get<type::PointerType>().Stride();
  }

  expr.ptr->Visit(*this);
  ptr_idx = current;
  state.pop();

  // read offset value
  state.push({State::Read, {}});
  expr.offs->Visit(*this);
  offs_idx = current;
  state.pop();

  auto ptr_var = emitter.vars[ptr_idx];
  EmitPointerIntegralExpr<calyx::AddToPointer>(
          emitter.vars[offs_idx].type, stride, ptr_idx, stride, offs_idx
  );
  ptr_idx = current;

  if (state.top().first == State::Read || state.top().first == State::ConditionalBranch) {
    auto visitor = detail::EmitterTypeVisitor<detail::LoadFromPointerEmitter>(*this, { ptr_idx });
    visitor.Visit(expr.ptr->type->Deref());

    if (state.top().first == State::ConditionalBranch) {
      EmitConditionalBranchForCurrent();
    }
  }
  else {
    switch (state.top().first) {
      case State::Assign: {
        auto var = state.top().second.var;
        auto visitor = detail::EmitterTypeVisitor<detail::StoreToPointerEmitter>(*this, { ptr_idx, var });
        visitor.Visit(expr.ptr->type->Deref());

        // stored value is "returned" value
        current = var;
        break;
      }
      case State::Address: {
        // current now holds the address already
        break;
      }
      default: {
        throw EmitterError("Bad state");
      }
    }
  }
}

void ASTWalker::Visit(const FunctionCallNode& expr) {
  state.push({State::Read, {}});
  expr.left->Visit(*this);
  // keep reading for arguments

  // regardless of whether the left type is a pointer to a function
  // or a function itself, "current" will hold a pointer to the
  // to-be-called address, as addresses are taken of function types
  const auto fn = current;
  const type::FunctionType& signature = expr.left->type.holds_alternative<type::FunctionType>() ? 
      expr.left->type.get<type::FunctionType>() :
      expr.left->type.get<type::PointerType>().contained->get<type::FunctionType>();

  const auto num_args = signature.arg_types.size();
  calyx::ArgData args{};

  for (int i = 0; i < num_args; i++) {
    expr.args[i]->Visit(*this);
    auto cast_arg_visitor = detail::EmitterTypeVisitor<detail::CastToEmitter>(*this, { current });
    cast_arg_visitor.Visit(*signature.arg_types[i].type);

    auto arg = detail::MakeLocal(0, *signature.arg_types[i].type);
    if (arg.type == calyx::Local::Type::Aggregate) {
      throw cotyl::UnimplementedException("Aggregate function argument type");
    }
    arg.non_aggregate.arg_idx = i;
    args.args.emplace_back(current, std::move(arg));
  }

  if (signature.variadic) {
    for (int i = num_args; i < expr.args.size(); i++) {
      expr.args[i]->Visit(*this);

      auto arg = detail::MakeLocal(0, expr.args[i]->type);
      if (arg.type == calyx::Local::Type::Aggregate) {
        throw cotyl::UnimplementedException("Aggregate function argument type");
      }

      args.var_args.emplace_back(current, std::move(arg));
    }
  }

  // stop reading for arguments
  state.pop();

  if (signature.contained->holds_alternative<type::VoidType>()) {
    emitter.Emit<calyx::Call<void>>(var_index_t{0}, fn, std::move(args));
  }
  else {
    auto visitor = detail::EmitterTypeVisitor<detail::CallEmitter>(*this, { fn, std::move(args) });
    visitor.Visit(*signature.contained);

    if (state.top().first == State::ConditionalBranch) {
      EmitConditionalBranchForCurrent();
    }
  }
}

void ASTWalker::Visit(const MemberAccessNode& expr) {
  if (state.top().first == State::Empty) {
    // increments might still happen
    expr.left->Visit(*this);
    return;
  }

  state.push({State::Address, {}});
  expr.left->Visit(*this);
  state.pop();

  // struct pointer
  auto strct  = current;

  // field offset
  auto offset = expr.left->type.visit<u64>(
    swl::overloaded{
      [&](const type::StructType& strct) { 
        return strct.MemberOffset(expr.member); 
      },
      [&](const type::UnionType& strct) { 
        return strct.MemberOffset(expr.member); 
      },
      [](const auto&) -> u64 {
        // already handled in parser
        throw cotyl::UnreachableException();
      }
    }
  );

  // field address
  auto imm = emitter.EmitExpr<calyx::Imm<u64>>({ Emitter::Var::Type::I32 }, offset);
  EmitPointerIntegralExpr<calyx::AddToPointer>(Emitter::Var::Type::U64, 1, strct, 1, imm);
  auto field = current;

  switch (state.top().first) {
    case State::Address: {
      // already done, current is now address of field
      break;
    }
    case State::Read:
    case State::ConditionalBranch: {
      auto visitor = detail::EmitterTypeVisitor<detail::LoadFromPointerEmitter>(*this, { current });
      visitor.Visit(expr.type);
      if (state.top().first == State::ConditionalBranch) {
        EmitConditionalBranchForCurrent();
      }
      break;
    }
    case State::Assign: {
      auto var = state.top().second.var;
      auto visitor = detail::EmitterTypeVisitor<detail::StoreToPointerEmitter>(*this, { current, var });
      visitor.Visit(expr.type);

      // stored value is "returned" value
      current = var;
      break;
    }
    case State::Empty: // already handled above
    default: {
      throw cotyl::UnreachableException();
    }
  }
}

void ASTWalker::Visit(const TypeInitializerNode& expr) {
  throw cotyl::UnimplementedException("type initializer");
}

void ASTWalker::Visit(const PostFixNode& expr) {
  cotyl::Assert(state.top().first == State::Empty || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  
  // block post updates on read, as they will be
  // handled on writeback
  state.push({State::Read, {}}); block_post_update++;
  expr.left->Visit(*this);
  state.pop(); block_post_update--;

  // don't increment if we don't handle post updates
  if (!block_post_update) {
    auto read = current;
    auto type = emitter.vars[current].type;
    if (type == Emitter::Var::Type::Pointer) {
      auto var = emitter.vars[read];
      auto value = expr.op == TokenType::Incr ? 1 : -1;
      auto imm = emitter.EmitExpr<calyx::Imm<i32>>({ Emitter::Var::Type::I32 }, value);
      EmitPointerIntegralExpr<calyx::AddToPointer>(Emitter::Var::Type::I32, var.stride, read, var.stride, imm);
    }
    else {
      EmitArithExpr<calyx::Imm>(type, 1);
      auto imm = current;
      auto op = expr.op == TokenType::Incr ? calyx::BinopType::Add : calyx::BinopType::Sub;
      EmitArithExpr<calyx::Binop>(type, read, op, imm);
    }

    // block pre updates on writeback, as they have been handled on read
    state.push({State::Assign, {.var = current}}); block_pre_update++;
    expr.left->Visit(*this);
    state.pop(); block_pre_update--;

    // restore read value for next expression
    current = read;
  }

  if (state.top().first == State::ConditionalBranch) {
    EmitConditionalBranchForCurrent();
  }
}

void ASTWalker::Visit(const UnopNode& expr) {
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
      // return, no need to check for conditional branches, is handled in visiting the 
      // the truthiness for the expression won't change for this unop
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
      
      if (conditional_branch) {
        // emit conditional branch with opposite destinations
        std::swap(state.top().second.true_block, state.top().second.false_block);
        EmitConditionalBranchForCurrent();
      }

      // return, no need to check for conditional branches, already handled
      return;
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
        // emit conditional branch with opposite destinations
        std::swap(state.top().second.true_block, state.top().second.false_block);
        EmitConditionalBranchForCurrent();
      }
      else {
        auto left = current;
        EmitArithExpr<calyx::Imm>(emitter.vars[left].type, 0);
        auto imm = current;
        EmitCompare<calyx::Compare>(emitter.vars[left].type, left, calyx::CmpType::Eq, imm);
      }

      // return, no need to check for conditional branches, already handled
      return;
    }
    case TokenType::Incr:
    case TokenType::Decr: {
      // block post-update on read, as explained in ASTWalker class
      state.push({State::Read, {}}); block_post_update++;
      expr.left->Visit(*this);
      state.pop(); block_post_update--;

      // pre-updates blocked, return read value
      // break to check for conditional branching
      if (block_pre_update) break;;

      auto type = emitter.vars[current].type;
      auto value = expr.op == TokenType::Incr ? 1 : -1;
      var_index_t stored;
      if (type == Emitter::Var::Type::Pointer) {
        auto left = current;
        auto var = emitter.vars[left];
        auto imm = emitter.EmitExpr<calyx::Imm<i32>>({ Emitter::Var::Type::I32 }, value);
        EmitPointerIntegralExpr<calyx::AddToPointer>(Emitter::Var::Type::I32, var.stride, left, var.stride, imm);
      }
      else {
        auto left = current;
        EmitArithExpr<calyx::Imm>(type, value);
        auto imm = current;
        EmitArithExpr<calyx::Binop>(type, left, calyx::BinopType::Add, imm);
      }
      stored = current;

      // block pre-updates on write back, have already been handled
      state.push({State::Assign, {.var = current}}); block_pre_update++;
      expr.left->Visit(*this);
      state.pop(); block_pre_update--;

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

        if (expr.type.holds_alternative<type::StructType>() || expr.type.holds_alternative<type::UnionType>()) {
          throw cotyl::UnimplementedException("Struct memcpy on assign");
        }
        auto var = state.top().second.var;
        auto visitor = detail::EmitterTypeVisitor<detail::StoreToPointerEmitter>(*this, { current, var });
        visitor.Visit(expr.type);

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

        auto deref_type = expr.left->type->Deref();
        // if dereferenced type is a function, don't dereference it
        // current will be the pointer itself
        // likely used for calls, i.e.
        // result = (*func)(a0, a1, a2)
        if (deref_type.holds_alternative<type::FunctionType>()) {
          // do nothing
        }
        else if (deref_type.holds_alternative<type::ArrayType>()) {
          // do nothing
        }
        else {
          auto visitor = detail::EmitterTypeVisitor<detail::LoadFromPointerEmitter>(*this, { current });
          visitor.Visit(deref_type);
        }

        // could be a conditional branch
      }
      break;
    }
    default: {
      throw EmitterError("Bad unop");
    }
  }

  if (conditional_branch) {
    EmitConditionalBranchForCurrent();
  }
}

void ASTWalker::Visit(const CastNode& expr) {
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
    visitor.Visit(expr.type);
  }
}

void ASTWalker::Visit(const BinopNode& expr) {
  if (state.top().first == State::Empty) {
    // increments / decrements might happen on either side
    expr.left->Visit(*this);
    expr.right->Visit(*this);
    return;
  }
  cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  
  // only need to push a new state for conditional branches
  const bool conditional_branch = state.top().first == State::ConditionalBranch;
  var_index_t left, right;

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
          // cast left and right to i64
          // static const auto ptrdiff_type = type::ValueType<i64>(type::LValue::None);
          
          // detail::EmitterTypeVisitor<detail::CastToEmitter>(*this, { left }).Visit(ptrdiff_type);
          // auto i64_left = current;
          // detail::EmitterTypeVisitor<detail::CastToEmitter>(*this, { right }).Visit(ptrdiff_type);
          // auto i64_right = current;

          // no need to cast at this point
          // EmitArithExpr<calyx::Binop>(Emitter::Var::Type::I64, i64_left, calyx::BinopType::Sub, i64_right);
          throw cotyl::UnimplementedException("Pointer diff");
        }
        else {
          EmitIntegralExpr<calyx::Unop>(emitter.vars[right].type, calyx::UnopType::Neg, right);
          EmitPointerIntegralExpr<calyx::AddToPointer>(emitter.vars[right].type, var.stride, left, var.stride, current);
        }
      }
      else if (emitter.vars[right].type == Emitter::Var::Type::Pointer) {
        throw EmitterError("Invalid right hand side operator for -: pointer");
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
        case TokenType::Less:         cmp_type = calyx::CmpType::Lt; break;
        case TokenType::LessEqual:    cmp_type = calyx::CmpType::Le; break;
        case TokenType::GreaterEqual: cmp_type = calyx::CmpType::Ge; break;
        case TokenType::Greater:      cmp_type = calyx::CmpType::Gt; break;
        case TokenType::Equal:        cmp_type = calyx::CmpType::Eq; break;
        case TokenType::NotEqual:     cmp_type = calyx::CmpType::Ne; break;
        default: throw cotyl::UnreachableException();
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
      cotyl::Assert(!cotyl::Is(emitter.vars[left].type).AnyOf<Emitter::Var::Type::Float, Emitter::Var::Type::Double, Emitter::Var::Type::Pointer>());
      cotyl::Assert(!cotyl::Is(emitter.vars[right].type).AnyOf<Emitter::Var::Type::Float, Emitter::Var::Type::Double, Emitter::Var::Type::Pointer>());
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
          throw EmitterError("Bad operand type for shift amount");
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
        auto name = cotyl::CString("$logop" + std::to_string(emitter.c_counter));
        auto [c_idx, _] = AddLocal(std::move(name), type::BoolType(type::LValue::Assignable));
        
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
      throw EmitterError("Bad binop");
  }

  if (conditional_branch) {
    EmitConditionalBranchForCurrent();
  }
}

void ASTWalker::Visit(const TernaryNode& expr) {
  cotyl::Assert(state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  
  if (state.top().first == State::Read) {
    // we create a "fake local" in order to do this
    auto name = cotyl::CString("$tern" + std::to_string(emitter.c_counter));
    auto type = expr.type;
    auto [c_idx, _] = AddLocal(std::move(name), type);

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
      store_visitor.Visit(type);
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
      store_visitor.Visit(type);
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
      load_visitor.Visit(type);
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

void ASTWalker::Visit(const AssignmentNode& expr) {
  cotyl::Assert(state.top().first == State::Empty || state.top().first == State::Read);
  bool conditional_branch = state.top().first == State::ConditionalBranch;

  if (block_pre_update) {
    // in-place assignments count as pre-updates
    // (e.g. int x = 0; int y = x += 1; assigns 2 to y)
    // if pre-updates are blocked, it means they have already happened
    // so we read the value of the assigned expression instead
    state.push({State::Read, {}});
    expr.left->Visit(*this);
    state.pop();

    if (conditional_branch) EmitConditionalBranchForCurrent();
    return;
  }
  
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

      // as explained in the ASTWalker class, we block
      // post updates when reading the expression to be updated
      state.push({State::Read, {}}); block_post_update++;
      expr.left->Visit(*this);
      state.pop(); block_post_update--;

      auto left = current;
      cotyl::Assert(!cotyl::Is(emitter.vars[left].type).AnyOf<Emitter::Var::Type::Float, Emitter::Var::Type::Double, Emitter::Var::Type::Pointer>());
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
          throw EmitterError("Bad operand type for shift amount");
        }
      }
      if (expr.op == TokenType::ILShift) {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Left, right);
      }
      else {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Right, right);
      }

      // do assignment, block pre-updates as explained
      auto assigned = current;
      state.push({State::Assign, {.var = assigned}}); block_pre_update++;
      expr.left->Visit(*this);
      state.pop(); block_pre_update--;
      current = assigned;

      if (conditional_branch) EmitConditionalBranchForCurrent();
      return;
    }
    case TokenType::IAnd: op = calyx::BinopType::BinAnd; break;
    case TokenType::IOr: op = calyx::BinopType::BinOr; break;
    case TokenType::IXor: op = calyx::BinopType::BinXor; break;
    case TokenType::Assign: {
      // don't block updates on a direct assignment
      // left-side expression is visited only once
      state.push({State::Assign, {.var = right}});
      expr.left->Visit(*this);
      state.pop();

      current = right;
      if (conditional_branch) EmitConditionalBranchForCurrent();
      return;
    }
    default: {
      throw EmitterError("Invalid assignment statement");
    }
  }

  // visit left for binop assignment
  state.push({State::Read, {}}); block_post_update++;
  expr.left->Visit(*this);
  state.pop(); block_post_update--;

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
        throw EmitterError("Bad pointer binop in pointer assignment");
    }
  }
  else {
    BinopHelper(left, op, right);
  }

  auto result = current;

  // do assignment
  state.push({State::Assign, {.var = current}}); block_pre_update++;
  expr.left->Visit(*this);
  state.pop(); block_pre_update--;

  // set current to the result variable
  current = result;
  if (conditional_branch) EmitConditionalBranchForCurrent();
}

void ASTWalker::Visit(const ExpressionListNode& expr) {
  // state does not matter for first nodes (cast to void)
  // this gets rid of any statements that have no effect
  state.push({State::Empty, {}});
  for (size_t i = 0; i < expr.exprs.size() - 1; i++) {
    expr.exprs.at(i)->Visit(*this);
  }
  state.pop();

  // visit last expression with current state
  expr.exprs.back()->Visit(*this);
}

}
