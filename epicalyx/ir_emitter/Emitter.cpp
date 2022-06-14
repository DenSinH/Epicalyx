#include "Emitter.h"
#include "ASTWalker.h"


namespace epi {

Emitter::Emitter() {

}

void Emitter::MakeProgram(std::vector<ast::pNode<ast::Decl>>& ast) {
  auto walker = ASTWalker(*this);
  for (auto& decl : ast) {
    decl->Visit(walker);
  }
}

}