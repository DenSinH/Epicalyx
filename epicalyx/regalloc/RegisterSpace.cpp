#include "RegisterSpace.h"


namespace epi {

using namespace calyx;

void RegisterSpace::EmitFunction(const Function& function) {
  // argument locals may not be accessed, but may be analyzed
  for (const auto& [loc_idx, loc] : function.locals) {
    Emit(loc_idx, loc);
  }

  for (const auto& [block_idx, block] : function.blocks) {
    for (const auto& dir : block) {
      Emit(dir);
    }
  }
}

}