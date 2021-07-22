#include "Statement.h"
#include "Declaration.h"

#include <regex>
#include <sstream>

namespace epi {

std::string If::to_string() const {
  if (_else) {
    return cotyl::FormatStr("if (%s) %s\nelse %s", cond, stat, _else);
  }
  return cotyl::FormatStr("if (%s) %s", cond, stat);
}

std::string While::to_string() const {
  return cotyl::FormatStr("while (%s) %s", cond, stat);
}

std::string DoWhile::to_string() const {
  return cotyl::FormatStr("do %s while (%s);", stat, cond);
}

std::string Return::to_string() const {
  if (expr) {
    return cotyl::FormatStr("return %s;", expr);
  }
  return "return;";
}

std::string For::to_string() const {
  std::stringstream result{};
  result << "for (";
  result << cotyl::Join(", ", decl);
  result << cotyl::Join(", ", init);
  result << "; ";
  result << cond->to_string();
  result << "; ";
  result << cotyl::Join(", ", update);
  result << ") ";
  result << stat->to_string();
  return result.str();
}

std::string Compound::to_string() const {
  std::stringstream repr{};
  repr << '{';
  for (const auto& stat : stats) {
    repr << '\n';
    repr << stat->to_string();
    if (stat->IsDeclaration()) {
      repr << ';';
    }
  }
  std::string result = std::regex_replace(repr.str(), std::regex("\n"), "\n  ");
  return result + "\n}";
}

}