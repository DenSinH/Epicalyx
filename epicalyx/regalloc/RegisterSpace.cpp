#include "RegisterSpace.h"


namespace epi {

using namespace calyx;

void RegisterSpace::EmitFunction(const Function& function) {
  for (const auto& [block_idx, block] : function.blocks) {
    for (const auto& dir : block) {
      dir->Emit(*this);
    }
  }
}

}