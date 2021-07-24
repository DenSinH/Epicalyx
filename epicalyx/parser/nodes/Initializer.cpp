#include "Initializer.h"

#include <sstream>
#include <regex>

namespace epi {


std::string InitializerList::to_string() const {
  std::stringstream repr{};
  repr << '{';
  for (const auto& init : list) {
    repr << '\n';
    for (const auto& des : init.first) {
      if (std::holds_alternative<std::string>(des)) {
        repr << '.' << std::get<std::string>(des);
      }
      else {
        repr << '[' << std::get<i32>(des) << ']';
      }
    }
    if (!init.first.empty()) {
      repr << " = ";
    }
    if (std::holds_alternative<pExpr>(init.second)) {
      repr << std::get<pExpr>(init.second)->to_string() << ',';
    }
    else {
      repr << std::get<pNode<InitializerList>>(init.second)->to_string() << ',';
    }
  }
  std::string result = std::regex_replace(repr.str(), std::regex("\n"), "\n  ");
  return result + "\n}";
}


}