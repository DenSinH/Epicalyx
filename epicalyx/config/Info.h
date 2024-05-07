#pragma once

#include <string>

namespace epi::info {

struct ProgramSettings {
  std::string filename;
  std::string stl;
  
  std::string rigfunc;
  bool novisualize;
  bool catch_errors;
};

void variant_sizes();
ProgramSettings parse_args(int argc, char** argv);

}