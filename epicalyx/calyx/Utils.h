#pragma once

#include "Default.h"
#include "Types.h"

namespace epi::calyx {

struct Function;

void InterpretGlobalInitializer(Global& dest, Function&& func);

}