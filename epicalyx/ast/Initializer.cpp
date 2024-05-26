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

void Initializer::ValidateAndReduce(type::AnyType& type) {
  if (swl::holds_alternative<InitializerList>(value)) {
    auto& list = swl::get<InitializerList>(value);
    list.ValidateAndReduce(type);

    // since we first validate and reduce the initializer list,
    // the nested initializers will already be reduced
    // this means we do not need to recurse the reduction
    // (for obtaining constant values / type values from initializer lists)
    type.visit<void>(
      [](const type::StructType& strct) {
        throw cotyl::UnimplementedException();
      },
      [](const type::UnionType& strct) {
        throw cotyl::UnimplementedException();
      },
      [](const type::ArrayType& arr) {
        // all reductions are done in the recursion above
      },
      [](const type::VoidType&) {
        throw type::TypeError("Initializer list for incomplete type");
      },
      [&](const type::PointerType& ptr) { 
        if (list.list.empty()) return;  // nullptr
        // list wil have size one by recursion above
        cotyl::Assert(list.list.size() == 1);
        value = std::move(list.list[0].second.value);
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
      swl::exhaustive
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
        // try to cast
        type.Cast(has, false);
      },
      [&](type::ArrayType& arr) {
        // for string constants, which have array type
        if (!has.holds_alternative<type::ArrayType>()) {
          throw type::TypeError("Invalid type for array initializer");
        }
        if (!arr.size) {
          arr.size = has.get<type::ArrayType>().size;
        }
        if (!type.TypeEquals(has)) {
          throw cotyl::FormatExceptStr<type::TypeError>("Cannot cast type %s to %s in initializer", has, type);
        }
      },
      [&]<typename T>(const type::ValueType<T>& val) {
        // function type, value type
        type.Cast(has);
      },
      [&](const type::FunctionType& val) {
        // function type, value type
        if (!type.TypeEquals(has)) {
          throw cotyl::FormatExceptStr<type::TypeError>("Cannot cast type %s to %s in initializer", has, type);
        }
      },
      swl::exhaustive
    );
  }
}

std::string Initializer::ToString() const {
  if (swl::holds_alternative<pExpr>(value))
    return swl::get<pExpr>(value)->ToString();
  else
    return swl::get<InitializerList>(value).ToString();
}


void InitializerList::ValidateAndReduceScalarType(type::AnyType& type) {
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

void InitializerList::ValidateAndReduce(type::AnyType& type) {
  type.visit<void>(
    [](const type::StructType& strct) {
      throw cotyl::UnimplementedException();
    },
    [](const type::UnionType& strct) {
      throw cotyl::UnimplementedException();
    },
    [](const type::VoidType&) {
      throw type::TypeError("Initializer list for incomplete type");
    },
    [&](const type::PointerType& ptr) { 
      ValidateAndReduceScalarType(type);
    },
    [&](type::ArrayType& arr) {
      auto& nested = *arr.contained;
      i64 index = 0;
      i64 size = 0;

      // validate and reduce all designators
      // compute array size from designators
      for (auto& designator : list) {
        
        // designator overrides current index
        if (!designator.first.empty()) {
          // array, so expect array index designators ([0] = 1), etc.
          if (!swl::holds_alternative<i64>(designator.first[0])) {
            throw cotyl::UnimplementedException("Expected array index designator in array initializer list");
          }
          if (designator.first.size() > 1) {
            throw cotyl::UnimplementedException("Nested designators");
          }

          // get new index
          index = swl::get<i64>(designator.first[0]);
        }
        designator.second.ValidateAndReduce(nested);
        index++;

        if (arr.size && index > arr.size) {
          throw type::TypeError("Initializer list index out of bounds for array");
        }
        
        // track initializer list array size
        // use incremented index
        size = std::max(size, index);
      }

      // computed size from initializer list
      if (arr.size == 0) {
        arr.size = size;
      }
    },
    [&]<typename T>(const type::ValueType<T>& val) {
      ValidateAndReduceScalarType(type);
    },
    [&](const type::FunctionType& val) {
      ValidateAndReduceScalarType(type);
    },
    swl::exhaustive
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