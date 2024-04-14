#pragma once

#include "calyx/Directive.h"

namespace epi {

template<typename T>
static bool IsType(const calyx::AnyDirective& directive) {
  return directive->type_id == T::GetTID();
}

static bool IsSameType(const calyx::AnyDirective& dir1, const calyx::AnyDirective& dir2) {
  return dir1->type_id == dir2->type_id;
}

}