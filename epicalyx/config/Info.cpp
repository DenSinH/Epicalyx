#include "Info.h"

#include "tokenizer/Token.h"
#include "calyx/Directive.h"
#include "types/AnyType.h"

#include "TypeName.h"

#include <iostream>


namespace epi::info {

template<typename T>
struct size_debugger;

template<typename P, typename... Ts>
struct size_debugger<epi::cotyl::Variant<P, Ts...>> {
  static void debug() {
    ((std::cout << epi::cotyl::type_name<Ts>() << ": " << sizeof(Ts) << " / "  << alignof(Ts) << std::endl), ...);
  }
};

void variant_sizes() {
  size_debugger<calyx::detail::any_directive_t>::debug();
  std::cout << "AnyDirective: " << sizeof(calyx::AnyDirective) << " / "  << alignof(calyx::AnyDirective) << std::endl;
  size_debugger<detail::any_token_t>::debug();
  std::cout << "AnyToken: " << sizeof(AnyToken) << " / "  << alignof(AnyToken) << std::endl;
  size_debugger<type::detail::any_type_t>::debug();
  std::cout << "AnyType: " << sizeof(type::AnyType) << " / "  << alignof(type::AnyType) << std::endl;
}

}