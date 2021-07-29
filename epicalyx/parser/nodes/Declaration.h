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

struct Declaration final : public Decl {

  Declaration(pType<> type, std::string name, StorageClass storage = StorageClass::Auto, std::optional<Initializer> value = {}) :
      name(std::move(name)),
      type(std::move(type)),
      storage(storage),
      value(std::move(value)) {

  }

  std::string name;
  pType<const CType> type;  // empty if abstract
  StorageClass storage;
  std::optional<Initializer> value;

  std::string ToString() const final;
  void VerifyAndRecord(Parser& parser) override;
  void DReduce(const Parser &parser);
};

struct FunctionDefinition final : public Decl {

  FunctionDefinition(pType<const FunctionType> signature, std::string symbol, pNode<Compound>&& body) ;

  pType<const FunctionType> signature;
  const std::string symbol;
  pNode<Compound> body;

  std::string ToString() const final;
  void VerifyAndRecord(Parser& parser) final;
};

}