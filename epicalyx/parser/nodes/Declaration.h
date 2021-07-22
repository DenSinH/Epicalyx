#pragma once

#include "Node.h"
#include "types/Types.h"
#include "Format.h"

#include <utility>

namespace epi {

enum StorageClass {
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
  pType<> type;  // empty if abstract
  StorageClass storage;

  std::string to_string() const override { return cotyl::FormatStr("%s %s", type, name); }
};

struct InitDeclaration : public Declaration {

  InitDeclaration(pNode<Declaration>&& decl, pExpr value) :
          Declaration(*decl),
          value(std::move(value)) {

  }

  pExpr value;

  std::string to_string() const final { return cotyl::FormatStr("%s %s = %s", type, name, value); }
};

}