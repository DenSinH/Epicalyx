#include "Expression.h"
#include <sstream>


namespace epi {

std::string FunctionCall::to_string() const {
  std::stringstream result{};
  result << left->to_string() << '(';
  for (int i = 0; i < args.size(); i++) {
    result << args[i]->to_string();
    if (i != args.size() - 1) {
      result << ", ";
    }
  }
  result << ')';
  return result.str();
}

}