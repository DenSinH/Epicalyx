#include "Expression.h"
#include <sstream>
#include <regex>


namespace epi {

std::string FunctionCall::to_string() const {
  std::stringstream result{};
  result << left->to_string() << "(";
  for (int i = 0; i < args.size(); i++) {
    result << args[i]->to_string();
    if (i != args.size() - 1) {
      result << ", ";
    }
  }
  return result.str();
}


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
    repr << init.second->to_string() << ',';
  }
  std::string result = std::regex_replace(repr.str(), std::regex("\n"), "\n  ");
  return result + "\n}";
}

}