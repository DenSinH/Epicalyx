#pragma once

#include "GeneralizedVar.h"
#include "cycle/Graph.h"
#include "Containers.h"

#include <string>

// Register Interference Graph

namespace epi {

using namespace calyx;

struct RIG {

  static RIG GenerateRIG(const Program& program);
  void Visualize(const std::string& filename) const;

  // we again skip 0
  Graph<i64, GeneralizedVar> graph{};
};

}