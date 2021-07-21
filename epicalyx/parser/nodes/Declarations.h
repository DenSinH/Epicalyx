#pragma once

#include "Node.h"
#include "types/Types.h"


namespace epi {

enum StorageClass {
  Typedef,
  Extern,
  Static,
  ThreadLocal,
  Register,
  Auto,
};

struct Declarator;
using AbstractDeclarator = Declarator;

struct Declarator : public Decl {

  Declarator(pType<> type, std::string name, StorageClass storage = StorageClass::Auto) :
      name(std::move(name)),
      type(std::move(type)),
      storage(storage) {

  }

  std::string name;
  pType<> type;  // empty if abstract
  StorageClass storage;
};

struct InitDeclarator : public Decl {

  InitDeclarator(pNode<Declarator>&& decl, pExpr&& value) :
      decl(std::move(decl)),
      value(std::move(value)) {

  }

  pNode<Declarator> decl;
  pExpr value;
};

}