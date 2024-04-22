#include "Declaration.h"
#include "Statement.h"

#include "types/Types.h"


namespace epi::ast {

DeclarationNode::DeclarationNode(type::AnyType&& type, cotyl::CString&& name, StorageClass storage, std::optional<Initializer>&& value) :
    name{std::move(name)}, type{std::move(type)}, storage{storage}, value{std::move(value)} {
  if (value.has_value()) {
    auto& init = this->value.value();
    init.ValidateAndReduce(type);
  }
}

std::string DeclarationNode::ToString() const {
  if (value.has_value()) {
    return cotyl::FormatStr("%s %s = %s", type, name.str(), value.value());
  }
  else if (!name.empty()) {
    return cotyl::FormatStr("%s %s", type, name.str());
  }
  return stringify(type);
}

FunctionDefinitionNode::FunctionDefinitionNode(type::FunctionType&& signature, cotyl::CString&& symbol, pNode<CompoundNode>&& body) :
    signature{std::move(signature)}, symbol{std::move(symbol)}, body{std::move(body)} { 

}

std::string FunctionDefinitionNode::ToString() const {
  return cotyl::FormatStr("%s %s %s", signature, symbol.str(), body);
}

}