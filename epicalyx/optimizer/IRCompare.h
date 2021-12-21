#pragma once

#include "calyx/backend/Backend.h"

#include <vector>
#include <unordered_map>

namespace epi {

using namespace calyx;

template<typename T>
bool IsType(pDirective& directive) {
  return directive->type_id == T::GetTID();
}

}