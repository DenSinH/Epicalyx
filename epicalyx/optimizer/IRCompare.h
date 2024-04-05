#pragma once

#include "calyx/backend/Backend.h"

namespace epi {

using namespace calyx;

template<typename T>
bool IsType(const pDirective& directive) {
  return directive->type_id == T::GetTID();
}

bool IsSameType(const pDirective& dir1, const pDirective& dir2) {
  return dir1->type_id == dir2->type_id;
}

}