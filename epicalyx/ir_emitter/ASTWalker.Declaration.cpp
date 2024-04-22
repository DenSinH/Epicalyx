#include "ASTWalker.h"
#include "Emitter.h"
#include "Helpers.h"

#include "types/Types.h"
#include "calyx/Calyx.h"
#include "calyx/Utils.h"
#include "ast/Statement.h"
#include "ast/Declaration.h"
#include "ast/Expression.h"

#include "Exceptions.h"
#include "CustomAssert.h"

#include "Helpers.inl"


namespace epi {

using namespace ast;


void ASTWalker::Visit(epi::DeclarationNode& decl) {
  if (locals.Depth() == 1) {
    // global symbols
    AddGlobal(decl.name, decl.type);

    auto global_type = detail::GetGlobalValue(decl.type);
    if (emitter.program.globals.contains(decl.name)) {
      throw cotyl::FormatExcept("Duplicate global symbol: %s", decl.name.c_str());
    }
    auto it = emitter.program.globals.emplace(decl.name, std::move(global_type));
    calyx::global_t& global = it.first->second;

    if (decl.value.has_value()) {
      if (std::holds_alternative<pExpr>(decl.value.value().value)) {
        auto& expr = std::get<pExpr>(decl.value.value().value);

        calyx::Function initializer{cotyl::CString("$init" + decl.name.str())};
        emitter.SetFunction(initializer);

        state.push({State::Read, {}});
        expr->Visit(*this);
        state.pop();

        auto global_block_return_visitor = detail::EmitterTypeVisitor<detail::ReturnEmitter>(*this, { current });
        global_block_return_visitor.Visit(decl.type);

        calyx::InterpretGlobalInitializer(global, std::move(initializer));
      }
      else {
        // todo: handle initializer list
        throw cotyl::UnimplementedException("global initializer list declaration");
      }
    }
  }
  else {
    auto c_idx = AddLocal(cotyl::CString(decl.name), decl.type);

    if (decl.value.has_value()) {
      if (std::holds_alternative<pExpr>(decl.value.value().value)) {
        state.push({State::Read, {}});
        std::get<pExpr>(decl.value.value().value)->Visit(*this);
        state.pop();
        // current now holds the expression id that we want to assign with
        state.push({State::Assign, {.var = current}});
        IdentifierNode(std::move(decl.name), std::move(decl.type)).Visit(*this);
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
  NewFunction(std::move(decl.symbol), decl.signature);

  // same as normal compound statement besides arguments
  locals.NewLayer();
  for (int i = 0; i < decl.signature.arg_types.size(); i++) {
    // turn arguments into locals
    auto& arg = decl.signature.arg_types[i];
    AddLocal(cotyl::CString(arg.name), *arg.type, i);
  }

  // locals layer
  locals.NewLayer();

  function = &decl;
  for (const auto& node : decl.body->stats) {
    node->Visit(*this);
  }
  emitter.Emit<calyx::Return<void>>();
  // locals layer
  locals.PopLayer();

  // arguments layer
  locals.PopLayer();

  EndFunction();
}

}
