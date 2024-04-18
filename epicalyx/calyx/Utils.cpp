#include "Utils.h"
#include "Types.h"
#include "backend/interpreter/Interpreter.h"


namespace epi::calyx {

void InterpretGlobalInitializer(global_t& dest, Function&& func) {
  Interpreter interpreter{};
  interpreter.InterpretGlobalInitializer(dest, std::move(func));
}

}