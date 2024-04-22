#include "Initializer.h"
#include "Node.h"
#include "Expression.h"
#include "types/AnyType.h"

#include "Log.h"
#include "Exceptions.h"
#include "SStream.h"

#include <regex>

namespace epi::ast {

  
Initializer::Initializer() : value{InitializerList{}} {

}

Initializer::Initializer(pExpr&& expr) : 
    value{std::move(expr)} {

}
  
Initializer::Initializer(InitializerList&& init) :
    value{std::move(init)} {

}

void Initializer::ValidateAndReduce(const type::AnyType& type) {
  if (std::holds_alternative<InitializerList>(value)) {
    auto& list = std::get<InitializerList>(value);
    list.ValidateAndReduce(type);

    // since we first validate and reduce the initializer list,
    // the nested initializers will already be reduced
    // this means we do not need to recurse the reduction
    // (for obtaining constant values / type values from initializer lists)
    type.visit<void>(
      [&](const type::StructType& strct) {
        throw cotyl::UnimplementedException();
      },
      [&](const type::UnionType& strct) {
        throw cotyl::UnimplementedException();
      },
      [&](const type::VoidType&) {
        throw std::runtime_error("Initializer list for incomplete type");
      },
      [&](const type::PointerType& ptr) { 
        if (ptr.size == 0) {
          if (list.list.empty()) return;  // nullptr
          // list wil have size one by recursion above
          cotyl::Assert(list.list.size() == 1);
          value = std::move(list.list[0].second.value);
        }
        else {
          // not implemented
        }
      },
      [&](const auto& val) { 
        using value_t = std::decay_t<decltype(val)>;
        static_assert(std::is_same_v<value_t, type::FunctionType> || cotyl::is_instantiation_of_v(type::ValueType, value_t));
        if (list.list.empty()) {
          if constexpr(cotyl::is_instantiation_of_v<type::ValueType, value_t>) {
            value = std::make_unique<NumericalConstantNode<typename value_t::type_t>>(0);
          }
        }
        // list wil have size one by recursion above
        cotyl::Assert(list.list.size() == 1);
        value = std::move(list.list[0].second.value);
      }
    );
  }
  else {
    const auto& has = std::get<pExpr>(value)->type;
    
    type.visit<void>(
      [&](const type::StructType& strct) {
        throw cotyl::UnimplementedException();
      },
      [&](const type::UnionType& strct) {
        throw cotyl::UnimplementedException();
      },
      [&](const type::VoidType&) {
        throw std::runtime_error("Initializer for incomplete type");
      },
      [&](const type::PointerType& ptr) {
        if (ptr.size == 0) {
          // try to cast
          type.Cast(has);
        }
        else {
          // array type
          throw std::runtime_error("Expected initializer list");
        }
      },
      [&](const auto& val) {
        // function type, value type
        using value_t = std::decay_t<decltype(val)>;
        static_assert(std::is_same_v<value_t, type::FunctionType> || cotyl::is_instantiation_of_v(type::ValueType, value_t));
        if (!type.TypeEquals(has)) {
          throw cotyl::FormatExceptStr("Cannot cast type %s to %s in initializer", has, type);
        }
      }
    );
  }
}

std::string Initializer::ToString() const {
  if (std::holds_alternative<pExpr>(value))
    return std::get<pExpr>(value)->ToString();
  else
    return std::get<InitializerList>(value).ToString();
}


void InitializerList::ValidateAndReduceScalarType(const type::AnyType& type) {
  if (list.empty()) return;  // always valid
  if (list.size() > 1) {
    Log::Warn("Excess elements in initializer list");
  }
  if (!list[0].first.empty()) {
    throw std::runtime_error("Bad initializer list: no declarators expected");
  }
  list.resize(1);
  list[0].second.ValidateAndReduce(type);
}

void InitializerList::ValidateAndReduce(const type::AnyType& type) {
  type.visit<void>(
    [&](const type::StructType& strct) {
      throw cotyl::UnimplementedException();
    },
    [&](const type::UnionType& strct) {
      throw cotyl::UnimplementedException();
    },
    [&](const type::VoidType&) {
      throw std::runtime_error("Initializer list for incomplete type");
    },
    [&](const type::PointerType& ptr) { 
      if (ptr.size == 0) {
        ValidateAndReduceScalarType(type);
      }
      else {
        // not implemented
      }
    },
    [&](const auto& val) { 
      static_assert(std::is_same_v<value_t, type::FunctionType> || cotyl::is_instantiation_of_v(type::ValueType, value_t));
      ValidateAndReduceScalarType(type);
    }
  );
}

void InitializerList::Push(DesignatorList&& member, Initializer&& value) {
  list.emplace_back(std::move(member), std::move(value));
}

std::string InitializerList::ToString() const {
  cotyl::StringStream repr{};
  repr << '{';
  for (const auto& init : list) {
    repr << '\n';
    for (const auto& des : init.first) {
      if (std::holds_alternative<cotyl::CString>(des)) {
        repr << '.' << std::get<cotyl::CString>(des);
      }
      else {
        repr << '[' << std::to_string(std::get<i64>(des)) << ']';
      }
    }
    if (!init.first.empty()) {
      repr << " = ";
    }
    repr << stringify(init.second) << ',';
  }
  std::string result = std::regex_replace(repr.finalize(), std::regex("\n"), "\n  ");
  return result + "\n}";
}  

}