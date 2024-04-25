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

#include <iostream>


namespace epi {

using namespace ast;


void ASTWalker::AddGlobal(const cotyl::CString& symbol, const type::AnyType& type) {
  if (symbol_types.contains(symbol)) {
    throw cotyl::FormatExcept("Duplicate global symbol: %s", symbol.c_str());
  }
  symbol_types.emplace(symbol, type);
}

void ASTWalker::AssertClearState() {
  cotyl::Assert(local_labels.empty());
  cotyl::Assert(select_stack.empty());
  cotyl::Assert(continue_stack.empty());
  cotyl::Assert(break_stack.empty());
  cotyl::Assert(state.size() == 1 && state.top().first == State::Empty);
  cotyl::Assert(locals.Depth() == 1 && locals.Top().empty(), "Local scope is not empty at function start");
}

void ASTWalker::NewFunction(cotyl::CString&& symbol, const type::AnyType& type) {
  emitter.NewFunction(std::move(symbol));
  symbol_types.emplace(emitter.current_function->symbol, type);

  AssertClearState();
  locals.Clear();
}

void ASTWalker::EndFunction() {
  local_labels.clear();
  AssertClearState();
}

var_index_t ASTWalker::AddLocal(cotyl::CString&& name, const type::AnyType& type, std::optional<var_index_t> arg_index) {
  auto c_idx = emitter.c_counter++;
  // guarantees the type is complete
  size_t size = type->Sizeof();
  auto& loc = emitter.current_function->locals.emplace(c_idx, calyx::Local{detail::GetLocalType(type).first, c_idx, size, std::move(arg_index)}).first->second;
  locals.Set(std::move(name), LocalData{ &loc, type });
  return c_idx;
}

const type::AnyType& ASTWalker::GetSymbolType(const cotyl::CString& symbol) const {
  if (locals.Has(symbol)) {
    return locals.Get(symbol).type;
  }
  return symbol_types.at(symbol);
}

void ASTWalker::Visit(const epi::DeclarationNode& decl) {
  if (locals.Depth() == 1) {
    // global symbols
    AddGlobal(decl.name, decl.type);

    auto global_type = detail::GetGlobalValue(decl.type);
    if (emitter.program.globals.contains(decl.name)) {
      throw cotyl::FormatExcept("Duplicate global symbol: %s", decl.name.c_str());
    }
    auto it = emitter.program.globals.emplace(decl.name, std::move(global_type));
    calyx::Global& global = it.first->second;

    if (decl.value.has_value()) {
      if (swl::holds_alternative<pExpr>(decl.value.value().value)) {
        auto& expr = swl::get<pExpr>(decl.value.value().value);

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
    auto c_idx = AddLocal(cotyl::CString{decl.name}, decl.type);

    if (decl.value.has_value()) {
      if (swl::holds_alternative<pExpr>(decl.value.value().value)) {
        state.push({State::Read, {}});
        swl::get<pExpr>(decl.value.value().value)->Visit(*this);
        state.pop();
        // current now holds the expression id that we want to assign with
        state.push({State::Assign, {.var = current}});
        Visit(IdentifierNode(cotyl::CString{decl.name}, type::AnyType{decl.type}));
        state.pop();
      }
      else {
        // todo: handle initializer list
        throw cotyl::UnimplementedException("initializer list declaration");
      }
    }
  }
}

void ASTWalker::Visit(const FunctionDefinitionNode& decl) {
  NewFunction(cotyl::CString{decl.symbol}, decl.signature);

  // same as normal compound statement besides arguments
  locals.NewLayer();
  for (int i = 0; i < decl.signature.arg_types.size(); i++) {
    // turn arguments into locals
    auto& arg = decl.signature.arg_types[i];
    AddLocal(cotyl::CString{arg.name}, *arg.type, i);
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
