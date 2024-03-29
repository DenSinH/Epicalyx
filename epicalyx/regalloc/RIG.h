#pragma once

#include "calyx/Calyx.h"
#include "Containers.h"

// Register Interference Graph

namespace epi {

using namespace calyx;

struct RIG {

  static RIG GenerateRIG(const Program& program);

  struct GeneralizedVar {
    bool is_local;
    var_index_t idx;
  };

  // we again skip 0
  std::vector<GeneralizedVar> vars{{}};
};

}