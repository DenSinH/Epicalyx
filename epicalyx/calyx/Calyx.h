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

struct BasicBlock {

  std::size_t size() const { return directives.size(); }
  auto begin() { return directives.begin(); }
  auto end() { return directives.end(); }
  auto begin() const { return directives.begin(); }
  auto end() const { return directives.end(); }
  auto back() const { return directives.back(); }
  bool empty() const { return directives.empty(); }
  const AnyDirective& at(std::size_t index) const { return directives.at(index); }
  AnyDirective& at(std::size_t index) { return directives.at(index); }
  void reserve(std::size_t size) { directives.reserve(size); }
  void push_back(AnyDirective&& value);

  std::size_t RemoveNoOps();

private:
  std::vector<AnyDirective> directives{};
};


struct Function {
  static constexpr block_label_t Entry = 1;

  Function(const std::string& symbol) : symbol{symbol} { }

  std::string symbol;
  // program code
  // block 0 is special
  cotyl::unordered_map<block_label_t, BasicBlock> blocks{};

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
