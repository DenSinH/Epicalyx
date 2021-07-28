#include "Declaration.h"
#include "Statement.h"

#include "types/Types.h"
#include "parser/Parser.h"


namespace epi {

std::string InitDeclaration::to_string() const {
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


FunctionDefinition::FunctionDefinition(pType<const FunctionType> signature, std::string symbol, pNode<Compound>&& body) :
    signature(std::move(signature)),
    symbol(std::move(symbol)),
    body(std::move(body)) {

}

std::string FunctionDefinition::to_string() const {
  return cotyl::FormatStr("%s %s %s", signature, symbol, body);
}

void Declaration::VerifyAndRecord(Parser& parser) {
  if (!name.empty()) {
    if (parser.variables.HasTop(name)) {
      if (!parser.variables.Get(name)->EqualType(*type)) {
        throw cotyl::FormatExceptStr("Redefinition of symbol %s", name);
      }
    }
    else {
      parser.variables.Set(name, type);
    }
  }
}

void InitDeclaration::VerifyAndRecord(Parser& parser) {
  // todo: verify initializer

  Declaration::VerifyAndRecord(parser);
}

void FunctionDefinition::VerifyAndRecord(Parser& parser) {
  if (parser.variables.Has(symbol)) {
    if (!parser.variables.Get(symbol)->EqualType(*signature)) {
      throw cotyl::FormatExceptStr("Redefinition of function: %s", symbol);
    }
  }
  else {
    parser.variables.Set(symbol, signature);
  }
}

}