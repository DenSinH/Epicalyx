#include "Declaration.h"


namespace epi {

std::string InitDeclaration::to_string() const  {
  if (value.has_value()) {
    if (std::holds_alternative<pExpr>(value.value()))
      return cotyl::FormatStr("%s %s = %s", type, name, std::get<pExpr>(value.value()));
    else
      return cotyl::FormatStr("%s %s = %s", type, name, std::get<pNode<InitializerList>>(value.value()));
  }
  else if (!name.empty()) {
    return cotyl::FormatStr("%s %s", type, name);
  }
  return type->to_string();
}

}