#include "RegisterSpace.h"


namespace epi {

void RegisterSpace::EmitProgram(const Program& program) {
  for (const auto& [block_idx, block] : program.blocks) {
    for (const auto& dir : block) {
      dir->Emit(*this);
    }
  }
}

}