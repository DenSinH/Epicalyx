#pragma once

#include "Node.h"
#include "types/EpiCType.h"
#include "CString.h"
#include "Format.h"
#include "Initializer.h"
#include "NodeVisitor.h"

#include <utility>
#include <optional>


namespace epi::ast {

struct CompoundNode;

enum class StorageClass {
  None,
  Typedef,
  Extern,
  Static,
  ThreadLocal,
  Register,
  Auto,
};

struct DeclarationNode final : public DeclNode {

  DeclarationNode(pType<> type, cotyl::CString&& name, StorageClass storage = StorageClass::Auto, std::optional<Initializer> value = {}) :
      name(std::move(name)),
      type(std::move(type)),
      storage(storage),
      value(std::move(value)) {

  }

  cotyl::CString name;
  pType<const CType> type;  // empty if abstract
  StorageClass storage;
  std::optional<Initializer> value;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  void VerifyAndRecord(Parser& parser) override;
  void DReduce(const Parser &parser);
};

struct FunctionDefinitionNode final : public DeclNode {

  FunctionDefinitionNode(pType<const FunctionType> signature, cotyl::CString&& symbol, pNode<CompoundNode>&& body);

  pType<const FunctionType> signature;
  cotyl::CString symbol;
  pNode<CompoundNode> body;

  std::string ToString() const final;
  void VerifyAndRecord(Parser& parser) final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

}