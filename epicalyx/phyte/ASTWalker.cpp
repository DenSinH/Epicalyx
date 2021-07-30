#include "ASTWalker.h"
#include "Emitter.h"
#include "calyx/Calyx.h"
#include "taxy/Declaration.h"
#include "taxy/Statement.h"
#include "taxy/Expression.h"


namespace epi::phyte {

void ASTWalker::Visit(epi::taxy::Declaration& decl) {
  if (emitter.variables.Depth() == 1) {
    // global symbols
  }
  else {
    auto c_idx = emitter.c_counter++;
    u64 size = decl.type->Sizeof();
    emitter.variables.Set(decl.name, calyx::CVar{
            c_idx, calyx::CVar::Location::Either, size
    });
    emitter.Emit<calyx::IRAllocateCVar>(c_idx, size);

    if (decl.value.has_value()) {
      throw std::runtime_error("Unimplemented");
    }
  }
}

void ASTWalker::Visit(epi::taxy::FunctionDefinition& decl) {
  // same as normal compound statement besides arguments
  emitter.variables.NewLayer();
  // todo: arguments etc
  for (const auto& node : decl.body->stats) {
    node->Visit(*this);
  }
  for (const auto& var : emitter.variables.Top()) {
    emitter.Emit<calyx::IRDeallocateCVar>(var.second.idx, var.second.size);
  }
  emitter.variables.PopLayer();
}

void ASTWalker::Visit(Identifier& decl) {
  // after the AST, the only identifiers left are C variables
  switch (emitter.state.top()) {
    case Emitter::State::Read: {
      break;
    }
  }
  throw std::runtime_error("Unimplemented");
}

void ASTWalker::Visit(NumericalConstant<i8>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<u8>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<i16>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<u16>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<i32>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<u32>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<i64>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i64>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<u64>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u64>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<float>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<float>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<double>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<double>>(expr.value);
}

void ASTWalker::Visit(StringConstant& expr) {
  // load from rodata
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(ArrayAccess& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(FunctionCall& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(MemberAccess& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(TypeInitializer& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(PostFix& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Unary& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Cast& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Binop& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Ternary& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Assignment& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Empty& stat) {

}

void ASTWalker::Visit(If& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(While& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(DoWhile& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(For& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Label& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Switch& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Case& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Default& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Goto& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Return& stat) {
  if (stat.expr) {
    stat.expr->Visit(*this);
    emitter.Emit<calyx::IRReturn>(current);
  }
  else {
    emitter.Emit<calyx::IRReturn>(0);
  }
}

void ASTWalker::Visit(Break& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Continue& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Compound& stat) {
  emitter.variables.NewLayer();
  for (const auto& node : stat.stats) {
    node->Visit(*this);
  }
  for (const auto& var : emitter.variables.Top()) {
    emitter.Emit<calyx::IRDeallocateCVar>(var.second.idx, var.second.size);
  }
  emitter.variables.PopLayer();
}


}
