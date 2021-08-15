#include "Emitter.h"
#include "ASTWalker.h"


namespace epi::phyte {

Emitter::Emitter() {
  program.push_back({});
}

void Emitter::MakeProgram(std::vector<taxy::pNode<taxy::Decl>>& ast) {
  auto walker = ASTWalker(*this);
  for (auto& decl : ast) {
    decl->Visit(walker);
  }
}

}