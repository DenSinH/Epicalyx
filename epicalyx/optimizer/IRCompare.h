#pragma once

#include "calyx/Directive.h"

namespace epi {

template<typename T>
static bool IsType(const calyx::AnyDirective& directive) {
  return directive.holds_alternative<T>();
}

static bool IsSameType(const calyx::AnyDirective& dir1, const calyx::AnyDirective& dir2) {
  return dir1.index() == dir2.index();
}

}