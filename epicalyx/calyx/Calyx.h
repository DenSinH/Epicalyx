#pragma once

#include "Types.h"
#include "Directive.h"
#include "Containers.h"
#include "Stringify.h"

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace epi::calyx {

struct Directive;
using block_t = std::vector<AnyDirective>;
using global_t = std::variant<i8, u8, i16, u16, i32, u32, i64, u64, float, double, Pointer, label_offset_t>;

struct Function {
  static constexpr block_label_t Entry = 1;

  Function(const std::string& symbol) : symbol{symbol} { }

  std::string symbol;
  // program code
  // block 0 is special
  cotyl::unordered_map<block_label_t, block_t> blocks{};

  cotyl::unordered_map<var_index_t, Local> locals{};  
  
  size_t Hash() const;
};

struct Program {
  // function symbols -> entrypoint block ID
  cotyl::unordered_map<std::string, Function> functions{};

  // string constants
  std::vector<std::string> strings{};

  // global variable sizes
  cotyl::unordered_map<std::string, global_t> globals{};

  size_t Hash() const;
};

void VisualizeProgram(const Program& program, const std::string& filename);
void PrintProgram(const Program& program);

}
