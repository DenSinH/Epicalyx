#include "Declaration.h"
#include "Statement.h"

#include "types/Types.h"


namespace epi::ast {

DeclarationNode::DeclarationNode(type::AnyType&& type, cotyl::CString&& name, StorageClass storage = StorageClass::Auto, std::optional<Initializer> value = {}) :
    name{std::move(name)}, type{std::move(type)}, storage{storage}, value{std::move(value)} { }

pNode<> DeclarationNode::Reduce() {
  if (value.has_value()) {
    auto& init = value.value();
    init.Reduce();
  }
  return nullptr;
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

FunctionDefinitionNode::FunctionDefinitionNode(type::AnyType&& signature, cotyl::CString&& symbol, pNode<CompoundNode>&& body) :
    signature{std::move(signature)}, symbol{std::move(symbol)}, body{std::move(body)} { }

std::string FunctionDefinitionNode::ToString() const {
  return cotyl::FormatStr("%s %s %s", signature, symbol.str(), body);
}

// void DeclarationNode::VerifyAndRecord(Parser& parser) {
//   if (!name.empty()) {
//     // todo: check enum/struct/typdef
//     if (parser.variables.HasTop(name)) {
//       // gets the first scoped value (which will be the top one)
//       if (!parser.variables.Get(name)->EqualType(*type)) {
//         throw cotyl::FormatExceptStr("Redefinition of symbol %s", name);
//       }
//     }
//     else {
//       parser.variables.Set(name, type);
//     }
//   }

//   if (value.has_value()) {
//     const auto& val = value.value();
//     if (holds_alternative<pExpr>(val)) {
//       type->Cast(*std::get<pExpr>(val)->GetType());
//     }
//     else {
//       auto visitor = ValidInitializerListVisitor(parser, *std::get<pNode<InitializerList>>(val));
//       type->Visit(visitor);
//     }
//   }
// }

// void FunctionDefinitionNode::VerifyAndRecord(Parser& parser) {
//   if (parser.variables.Has(symbol)) {
//     if (!parser.variables.Get(symbol)->EqualType(*signature)) {
//       throw cotyl::FormatExceptStr("Redefinition of function: %s", symbol);
//     }
//   }
//   else {
//     parser.variables.Set(symbol, signature);
//   }
// }

}