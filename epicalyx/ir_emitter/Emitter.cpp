#include "Emitter.h"
#include "ASTWalker.h"


namespace epi {

Emitter::Emitter() {

}

void Emitter::MakeProgram(cotyl::vector<ast::pNode<ast::DeclNode>>& ast) {
  auto walker = ASTWalker(*this);
  for (auto& decl : ast) {
    decl->Visit(walker);
  }
}

}