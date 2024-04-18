#pragma once

#include "Default.h"
#include "Types.h"
#include <variant>

namespace epi::calyx {

struct Function;

void InterpretGlobalInitializer(global_t& dest, Function&& func);

}