#include "Declaration.h"
#include "Statement.h"

#include "types/Types.h"
#include "parser/Parser.h"


namespace epi::ast {

FunctionDefinitionNode::FunctionDefinitionNode(pType<const FunctionType> signature, std::string symbol, pNode<CompoundNode>&& body) :
    signature(std::move(signature)),
    symbol(std::move(symbol)),
    body(std::move(body)) {

}

std::string DeclarationNode::ToString() const {
  if (value.has_value()) {
    if (std::holds_alternative<pExpr>(value.value()))
      return cotyl::FormatStr("%s %s = %s", type, name, std::get<pExpr>(value.value()));
    else
      return cotyl::FormatStr("%s %s = %s", type, name, std::get<pNode<InitializerList>>(value.value()));
  }
  else if (!name.empty()) {
    return cotyl::FormatStr("%s %s", type, name);
  }
  return type->ToString();
}

std::string FunctionDefinitionNode::ToString() const {
  return cotyl::FormatStr("%s %s %s", signature, symbol, body);
}

void DeclarationNode::VerifyAndRecord(Parser& parser) {
  if (!name.empty()) {
    // todo: check enum/struct/typdef
    if (parser.variables.HasTop(name)) {
      // gets the first scoped value (which will be the top one)
      if (!parser.variables.Get(name)->EqualType(*type)) {
        throw cotyl::FormatExceptStr("Redefinition of symbol %s", name);
      }
    }
    else {
      parser.variables.Set(name, type);
    }
  }

  if (value.has_value()) {
    const auto& val = value.value();
    if (holds_alternative<pExpr>(val)) {
      type->Cast(*std::get<pExpr>(val)->GetType());
    }
    else {
      auto visitor = ValidInitializerListVisitor(parser, *std::get<pNode<InitializerList>>(val));
      type->Visit(visitor);
    }
  }
}

void FunctionDefinitionNode::VerifyAndRecord(Parser& parser) {
  if (parser.variables.Has(symbol)) {
    if (!parser.variables.Get(symbol)->EqualType(*signature)) {
      throw cotyl::FormatExceptStr("Redefinition of function: %s", symbol);
    }
  }
  else {
    parser.variables.Set(symbol, signature);
  }
}

void DeclarationNode::DReduce(const Parser &parser) {
  if (value.has_value()) {
    auto& init = value.value();
    if (std::holds_alternative<pExpr>(init)) {
      auto n_expr = std::get<pExpr>(init)->EReduce(parser);
      if (n_expr) value = std::move(n_expr);
    }
    else {
      auto n_expr = ReduceInitializerListVisitor(parser, *std::get<pNode<InitializerList>>(init)).Reduce(*type);
      if (n_expr) value = std::move(n_expr);
    }
  }
}

}