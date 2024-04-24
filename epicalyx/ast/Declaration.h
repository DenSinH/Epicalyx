#pragma once

#include "Node.h"
#include "types/AnyType.h"
#include "CString.h"
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

struct DeclarationNode final : DeclNode {

  DeclarationNode(type::AnyType&& type, cotyl::CString&& name, StorageClass storage = StorageClass::Auto, std::optional<Initializer>&& value = {});
  
  cotyl::CString name;
  type::AnyType type;  // void if abstract
  StorageClass storage;
  std::optional<Initializer> value;

  std::string ToString() const final;
  VISIT_IMPL
};

struct FunctionDefinitionNode final : DeclNode {

  FunctionDefinitionNode(type::FunctionType&& signature, cotyl::CString&& symbol, pNode<CompoundNode>&& body);

  type::FunctionType signature;
  cotyl::CString symbol;
  pNode<CompoundNode> body;

  std::string ToString() const final;
  VISIT_IMPL
};

}