#include "Info.h"

#include "tokenizer/Token.h"
#include "calyx/Directive.h"
#include "types/AnyType.h"

#include "TypeName.h"

#include "argparse/argparse.hpp"
#include <iostream>
#include <cstdlib>


namespace epi::info {

  
ProgramSettings parse_args(int argc, char** argv) {
  ProgramSettings settings{};
  argparse::ArgumentParser program("epicalyx");
  program.add_argument("-variant-size")
         .help("Print large variant sizes and exit")
         .flag()
         .action([](const auto&) {
            variant_sizes();
            std::exit(0);
          })
          .nargs(0);
  program.add_argument("-novisualize")
         .help("Don't visualize program / RIG with graphviz")
         .flag()
         .store_into(settings.novisualize);
  program.add_argument("-rigfunc")
         .help("Function to analyze the RIG for")
         .metavar("FUNCTION")
         .default_value("main")
         .store_into(settings.rigfunc);
  program.add_argument("filename")
         .help("C file to compile")
         .metavar("FILENAME")
         .required()
         .store_into(settings.filename);
  
  try {
    program.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }
  
  return std::move(settings);
}

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