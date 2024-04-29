#include "Initializer.h"
#include "Node.h"
#include "Expression.h"
#include "types/AnyType.h"

#include "Log.h"
#include "Exceptions.h"
#include "SStream.h"
#include "Decltype.h"

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
  if (swl::holds_alternative<InitializerList>(value)) {
    auto& list = swl::get<InitializerList>(value);
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
        throw type::TypeError("Initializer list for incomplete type");
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
      [&]<typename T>(const type::ValueType<T>& val) { 
        if (list.list.empty()) {
          value = std::make_unique<NumericalConstantNode<T>>(0);
        }
        else {
          // list wil have size one by recursion above
          cotyl::Assert(list.list.size() == 1);
          value = std::move(list.list[0].second.value);
        }
      },
      [&](const type::FunctionType& val) { 
        if (list.list.empty()) {
          throw cotyl::UnimplementedException("Function pointer empty initializer list");
        }
        else {
          // list wil have size one by recursion above
          cotyl::Assert(list.list.size() == 1);
          value = std::move(list.list[0].second.value);
        }
      },
      // exhaustive variant access
      [](const auto& invalid) { static_assert(!sizeof(invalid)); }
    );
  }
  else {
    const auto& has = swl::get<pExpr>(value)->type;
    
    type.visit<void>(
      [&](const type::StructType& strct) {
        throw cotyl::UnimplementedException();
      },
      [&](const type::UnionType& strct) {
        throw cotyl::UnimplementedException();
      },
      [&](const type::VoidType&) {
        throw type::TypeError("Initializer for incomplete type");
      },
      [&](const type::PointerType& ptr) {
        if (ptr.size == 0) {
          // try to cast
          type.Cast(has, false);
        }
        else {
          // array type
          throw type::TypeError("Expected initializer list");
        }
      },
      [&]<typename T>(const type::ValueType<T>& val) {
        // function type, value type
        if (!type.TypeEquals(has)) {
          throw cotyl::FormatExceptStr<type::TypeError>("Cannot cast type %s to %s in initializer", has, type);
        }
      },
      [&](const type::FunctionType& val) {
        // function type, value type
        if (!type.TypeEquals(has)) {
          throw cotyl::FormatExceptStr<type::TypeError>("Cannot cast type %s to %s in initializer", has, type);
        }
      },
      // exhaustive variant access
      [](const auto& invalid) { static_assert(!sizeof(invalid)); }
    );
  }
}

std::string Initializer::ToString() const {
  if (swl::holds_alternative<pExpr>(value))
    return swl::get<pExpr>(value)->ToString();
  else
    return swl::get<InitializerList>(value).ToString();
}


void InitializerList::ValidateAndReduceScalarType(const type::AnyType& type) {
  if (list.empty()) return;  // always valid
  if (list.size() > 1) {
    Log::Warn("Excess elements in initializer list");
  }
  if (!list[0].first.empty()) {
    throw type::TypeError("Bad initializer list: no declarators expected");
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
      throw type::TypeError("Initializer list for incomplete type");
    },
    [&](const type::PointerType& ptr) { 
      if (ptr.size == 0) {
        ValidateAndReduceScalarType(type);
      }
      else {
        // not implemented
      }
    },
    [&]<typename T>(const type::ValueType<T>& val) {
      ValidateAndReduceScalarType(type);
    },
    [&](const type::FunctionType& val) {
      ValidateAndReduceScalarType(type);
    },
    // exhaustive variant access
    [](const auto& invalid) { static_assert(!sizeof(invalid)); }
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
      if (swl::holds_alternative<cotyl::CString>(des)) {
        repr << '.' << swl::get<cotyl::CString>(des);
      }
      else {
        repr << '[' << std::to_string(swl::get<i64>(des)) << ']';
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