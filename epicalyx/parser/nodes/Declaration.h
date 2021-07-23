#pragma once

#include "Node.h"
#include "types/Types.h"
#include "Format.h"
#include "Expression.h"

#include <utility>

namespace epi {

enum class StorageClass {
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
};

struct InitDeclaration : public Declaration {

  InitDeclaration(pNode<Declaration>&& decl, std::optional<Initializer> value = {}) :
          Declaration(*decl),
          value(std::move(value)) {

  }

  std::optional<Initializer> value;

  std::string to_string() const final;
};

}