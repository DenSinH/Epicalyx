#pragma once

#include "calyx/backend/Backend.h"

namespace epi {

template<typename T>
bool IsType(const calyx::pDirective& directive) {
  return directive->type_id == T::GetTID();
}

bool IsSameType(const calyx::pDirective& dir1, const calyx::pDirective& dir2) {
  return dir1->type_id == dir2->type_id;
}

}