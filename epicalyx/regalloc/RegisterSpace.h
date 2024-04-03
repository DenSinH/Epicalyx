#pragma once

#include "Default.h"
#include "GeneralizedVar.h"
#include "calyx/backend/Backend.h"

#include <memory>
#include <optional>


namespace epi {

using namespace calyx;

/*
 * Generic RegisterSpace class
 * This can be overridden to define the following:
 * - Register types (CPU, FPU) and a mapping from GeneralizedVars
 *   to them.
 *     These classify GeneralizedVars into clusters of potential
 *   overlapping variables. It also makes it so that certain
 *   variables can NOT overlap (i.e. a CPU and an FPU register).
 * - A mapping from register types to a register count.
 *     This gives the number of colors to color a RIG graph with for 
 *   register allocation.
 * - A mapping from GeneralizedVars to a forced register.
 *     These can be populated by overriding the proper 
 *   Backend::Emit methods, populating an unordered_map 
 *   mapping GeneralizedVars to potential forced registers.
 *   (for example needed for x86 integer division).
 * */

using register_type_t = u32;
using register_idx_t = u32;
using register_t = std::pair<register_type_t, register_idx_t>;

struct RegisterSpace : Backend {

  template<typename T, typename... Args>
  requires (std::is_base_of_v<RegisterSpace, T>)
  static std::unique_ptr<T> GetRegSpace(const Program& program, Args... args) {
    auto result = std::make_unique<T>(args...);
    result->EmitProgram(program);
    return std::move(result);
  }

  void EmitProgram(const Program& program) final;

  virtual register_type_t RegisterType(const GeneralizedVar& gvar) const = 0;
  virtual std::size_t RegisterTypePopulation(const register_type_t& type) const = 0;
  virtual std::optional<register_t> ForcedRegister(const GeneralizedVar& gvar) const = 0;
};

}