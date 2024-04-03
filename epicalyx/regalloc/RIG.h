#pragma once

#include "GeneralizedVar.h"
#include "cycle/Graph.h"
#include "Containers.h"

#include <string>

// Register Interference Graph

namespace epi {

using namespace calyx;
struct RegisterSpace;

struct RIG {

  static RIG GenerateRIG(const Program& program);
  void Reduce(const RegisterSpace& regspace);
  void Visualize(const std::string& filename) const;

  Graph<i64, GeneralizedVar, false> graph{};
};

}