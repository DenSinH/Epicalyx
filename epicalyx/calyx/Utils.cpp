#include "Utils.h"
#include "Types.h"
#include "backend/interpreter/Interpreter.h"


namespace epi::calyx {

void InterpretGlobalInitializer(Global& dest, Function&& func) {
  Interpreter interpreter{};
  interpreter.InterpretGlobalInitializer(dest, std::move(func));
}

}