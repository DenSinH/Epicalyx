#pragma once

#include "Node.h"
#include "types/EpiCType.h"
#include "Format.h"
#include "Expression.h"

#include <utility>
#include <optional>


namespace epi {

struct Compound;

enum class StorageClass {
  None,
  Typedef,
  Extern,
  Static,
  ThreadLocal,
  Register,
  Auto,
};

struct Declaration;
using AbstractDeclarator = Declaration;


struct Declaration : public Decl {

  Declaration(pType<> type, std::string name, StorageClass storage = StorageClass::Auto) :
      name(std::move(name)),
      type(std::move(type)),
      storage(storage) {

  }

  std::string name;
  pType<const CType> type;  // empty if abstract
  StorageClass storage;

  std::string to_string() const override { return cotyl::FormatStr("%s %s", type, name); }
  void VerifyAndRecord(Parser& parser) override;
};

struct InitDeclaration final : public Declaration {

  InitDeclaration(pNode<Declaration>&& decl, std::optional<Initializer> value = {}) :
          Declaration(*decl),
          value(std::move(value)) {

  }

  std::optional<Initializer> value;

  std::string to_string() const final;
  void VerifyAndRecord(Parser& parser) final;
  void DReduce(Parser &parser);
};

struct FunctionDefinition final : public Decl {

  FunctionDefinition(pType<const FunctionType> signature, std::string symbol, pNode<Compound>&& body) ;

  pType<const FunctionType> signature;
  const std::string symbol;
  pNode<Compound> body;

  std::string to_string() const final;
  void VerifyAndRecord(Parser& parser) final;
};

}