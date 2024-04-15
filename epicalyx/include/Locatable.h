#pragma once

namespace epi::cotyl {

struct Locatable {
  virtual void PrintLoc() const = 0;
};

}