#pragma once

#include "Types.h"
#include "Directive.h"
#include "Containers.h"
#include "Stringify.h"
#include "CString.h"

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
  const AnyDirective& back() const { return directives.back(); }
  AnyDirective& back() { return directives.back(); }
  bool empty() const { return directives.empty(); }
  const AnyDirective& at(std::size_t index) const { return directives.at(index); }
  AnyDirective& at(std::size_t index) { return directives.at(index); }
  void reserve(std::size_t size) { directives.reserve(size); }
  void push_back(AnyDirective&& value);

private:
  std::vector<AnyDirective> directives{};
};


struct Function {
  static constexpr block_label_t Entry = 1;

  Function(cotyl::CString&& symbol) : symbol{std::move(symbol)} { }

  cotyl::CString symbol;
  // program code
  // block 0 is special
  cotyl::unordered_map<block_label_t, BasicBlock> blocks{};

  cotyl::unordered_map<var_index_t, Local> locals{};  
  
  size_t Hash() const;
};

struct Program {
  // function symbols -> entrypoint block ID
  cotyl::unordered_map<cotyl::CString, Function> functions{};

  // string constants
  std::vector<cotyl::CString> strings{};

  // global variable sizes
  cotyl::unordered_map<cotyl::CString, global_t> globals{};

  size_t Hash() const;
};

void VisualizeProgram(const Program& program, const std::string& filename);
void PrintProgram(const Program& program);

}
