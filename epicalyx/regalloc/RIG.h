#pragma once

#include "cycle/Graph.h"
#include "calyx/Calyx.h"
#include "Containers.h"

// Register Interference Graph

namespace epi {

using namespace calyx;

struct RIG {

  static RIG GenerateRIG(const Program& program);
  void Visualize() const;

  struct GeneralizedVar {
    var_index_t idx;
    bool is_local = false;
    
    i64 NodeUID() const { return is_local ? -idx : idx; }
    auto operator<=>(const GeneralizedVar& other) const = default;
  };

  // we again skip 0
  Graph<i64, GeneralizedVar> graph{};
};

}