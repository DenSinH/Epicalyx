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

#undef DEBUG_GLOBAL_INITIALIZERS

#ifdef DEBUG_GLOBAL_INITIALIZERS
#include <iostream>
#endif

namespace epi {

using namespace ast;


#ifdef DEBUG_GLOBAL_INITIALIZERS
namespace calyx {

void VisualizeFunction(const Function& func, const std::string& filename);

}
#endif


void InitializeGlobalAggregate(u8* data, const type::AnyType& type, const Initializer& init);

void InitializeGlobalAggregate(u8* data, const type::AnyType& type, const InitializerList& list) {
  type.visit<void>(
    [&](const type::StructType& strct) {
      throw cotyl::UnimplementedException("Global struct aggregate initializer list");
    },
    [&](const type::UnionType& strct) {
      throw cotyl::UnimplementedException("Global union aggregate initializer list");
    },
    [&](const type::ArrayType& arr) {
      const auto stride = arr.Stride();
      cotyl::Assert(stride, "Initializing array without stride");

      // go through fields in incrementing order
      auto index = 0;
      const auto& contained = *arr.contained;
      for (const auto& designator : list.list) {
        // handle designators
        if (designator.first.size() > 1) {
          throw cotyl::UnimplementedException("Nested designators");
        }
        if (designator.first.size() == 1) {
          // designator was validated in initializer
          index = swl::get<i64>(designator.first[0]);
        }
        InitializeGlobalAggregate(data + stride * index, contained, designator.second);
        index++;
      }
    },
    [](const auto&) {
      // expected to be reduced to pExpr before
      throw cotyl::UnreachableException();
    }
  );
}

void InitializeGlobalAggregate(u8* data, const type::AnyType& type, const Initializer& init) {
  swl::visit(
    swl::overloaded{
      [&](const InitializerList& list) {
        InitializeGlobalAggregate(data, type, list);
      },
      [&](const pExpr& expr) {
        // cast expression to type
        auto casted = type.Cast(expr->type);
        casted.visit<void>(
          [](const type::PointerType&) {
            throw cotyl::UnimplementedException("Pointer in global initializer list");
          },
          [](const type::FunctionType&) {
            throw cotyl::UnimplementedException("Function pointer in global initializer list");
          },
          [&]<typename T>(const type::ValueType<T>& val) {
            if (!val.value.has_value()) {
              throw EmitterError("Expected constant value in global initializer list");
            }
            std::memcpy(data, &val.value.value(), sizeof(T));
          },
          [](const type::ArrayType&) {
            throw cotyl::UnimplementedException("Direct array (string constant) in global initializer list");
          },
          [](const auto&) {
            // cannot be an Expr
            throw cotyl::UnimplementedException("Direct struct value (memcpy) in global initializer");
          }
        );
      },
      swl::exhaustive
    },
    init.value
  );
}


void ASTWalker::AddGlobal(const cotyl::CString& symbol, const type::AnyType& type) {
  if (symbol_types.contains(symbol)) {
    // this is supposed to be checked in the parser
    cotyl::Assert(type.TypeEquals(symbol_types.at(symbol)));
  }
  else {
    symbol_types.emplace(symbol, type);
  }
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

std::pair<var_index_t, ASTWalker::LocalData> ASTWalker::AddLocal(cotyl::CString&& name, const type::AnyType& type, std::optional<var_index_t> arg_index) {
  auto c_idx = emitter.c_counter++;
  auto local = detail::MakeLocal(c_idx, type);
  if (arg_index.has_value()) {
    if (local.type == calyx::Local::Type::Aggregate) {
      throw cotyl::UnimplementedException("Aggregate argument types");
    }
    local.non_aggregate.arg_idx = std::move(arg_index.value());
  }
  auto& loc = emitter.current_function->locals.emplace(c_idx, std::move(local)).first->second;
  const auto& emplaced = locals.Set(std::move(name), LocalData{ &loc, type });
  return {c_idx, emplaced};
}

const type::AnyType& ASTWalker::GetSymbolType(const cotyl::CString& symbol) const {
  if (locals.Has(symbol)) {
    return locals.Get(symbol).type;
  }
  return symbol_types.at(symbol);
}

void ASTWalker::Visit(const epi::DeclarationNode& decl) {
  // function types forward declare global symbols within scopes
  if (locals.Depth() == 1 || decl.type.holds_alternative<type::FunctionType>()) {
    // global symbols
    AddGlobal(decl.name, decl.type);
    auto global_value = detail::GetGlobalValue(decl.type);

    // todo: return since previously initialized?
    // see 0098-tentative.c
    // if (emitter.program.globals.contains(decl.name)) {
    //   throw cotyl::FormatExcept("Duplicate global symbol: %s", decl.name.c_str());
    // }
    auto it = emitter.program.globals.emplace(decl.name, std::move(global_value));
    calyx::Global& global = it.first->second;

    if (decl.value.has_value()) {
      if (swl::holds_alternative<calyx::AggregateData>(global)) {
        auto& agg = swl::get<calyx::AggregateData>(global);
        InitializeGlobalAggregate(agg.data.get(), decl.type, decl.value.value());
      }
      else if (!swl::holds_alternative<pExpr>(decl.value.value().value)) {
        // initializer list expected to be reduced to pExpr in Parser
        throw cotyl::UnreachableException();
      }
      else {
        auto& expr = swl::get<pExpr>(decl.value.value().value);

        calyx::Function initializer{cotyl::CString("$init" + decl.name.str())};
        emitter.SetFunction(initializer);

        state.push({State::Read, {}});
        expr->Visit(*this);
        state.pop();

        auto global_block_return_visitor = detail::EmitterTypeVisitor<detail::ReturnEmitter>(*this, { current });
        global_block_return_visitor.Visit(decl.type);
#ifdef DEBUG_GLOBAL_INITIALIZERS
        std::cout << "Visualizing global initializer " << decl.name.c_str() << std::endl;
        VisualizeFunction(initializer, "output/init" + decl.name.str() + ".pdf");
#endif
        calyx::InterpretGlobalInitializer(global, std::move(initializer));
      }
    }
  }
  else {
    auto [c_idx, local] = AddLocal(cotyl::CString{decl.name}, decl.type);

    if (decl.value.has_value()) {
      if (local.loc->type == calyx::Local::Type::Aggregate) {
        throw cotyl::UnimplementedException("Local aggregate initializer");
      }
      else if (swl::holds_alternative<InitializerList>(decl.value.value().value)) {
        // initializer list expected to be reduced to pExpr in Parser
        throw cotyl::UnreachableException();
      }
      else {
        state.push({State::Read, {}});
        swl::get<pExpr>(decl.value.value().value)->Visit(*this);
        state.pop();
        // current now holds the expression id that we want to assign with
        state.push({State::Assign, {.var = current}});
        Visit(IdentifierNode(cotyl::CString{decl.name}, type::AnyType{decl.type}));
        state.pop();
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
