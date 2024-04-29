#pragma once

#include <ostream>


namespace epi::cotyl {

struct Locatable {
  virtual void PrintLoc(std::ostream& out) const = 0;
};

}