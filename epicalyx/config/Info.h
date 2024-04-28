#pragma once

#include <string>

namespace epi::info {

struct ProgramSettings {
  std::string filename;
  std::string rigfunc;
  bool novisualize;
};

void variant_sizes();
ProgramSettings parse_args(int argc, char** argv);

}