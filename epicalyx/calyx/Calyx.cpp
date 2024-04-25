#include "Calyx.h"
#include "Directive.h"
#include "cycle/Cycle.h"

#include "SStream.h"
#include "Format.h"
#include "Hash.h"
#include "Format.h"
#include "Exceptions.h"
#include "Decltype.h"

#include <iostream>
#include <tuple>


namespace epi::calyx {

BasicBlock::~BasicBlock() = default;

void BasicBlock::push_back(AnyDirective&& value) {
  directives.push_back(std::move(value));
}

void BasicBlock::reserve(std::size_t size) { 
  directives.reserve(size); 
}

std::pair<block_label_t, BasicBlock&> Function::AddBlock(block_label_t block_idx) {
  if (!block_idx) {
    // block 0 is special
    block_idx = blocks.size() + 1;
  }
  auto inserted = blocks.insert({block_idx, {}});
  cotyl::Assert(inserted.second, "Block already exists!");
  return {block_idx, inserted.first->second};
}

namespace detail {

std::string TypeString(const Local::Type& type);

}

size_t Function::Hash() const {
  size_t seed = blocks.size();
  cotyl::map<block_label_t, const BasicBlock&> sorted{blocks.begin(), blocks.end()};
  for (const auto& [block_idx, block] : sorted) {
    calyx::hash_combine(seed, block_idx);
    for (const auto& directive : block) {
      calyx::hash_combine(seed, directive.index());
    }
  }
  return seed;
}

size_t Program::Hash() const {
  size_t seed = functions.size();
  
  cotyl::map<std::string_view, const Function&> sorted{};
//   sorted.reserve(functions.size());
  for (const auto& [sym, func] : functions) {
    sorted.emplace(sym.view(), func);
  }

  for (const auto& [sym, func] : sorted) {
    calyx::hash_combine(seed, func.Hash());
  }
  return seed;
}


void PrintFunction(const Function& func) {
  for (const auto& [i, block] : func.blocks) {
    if (!block.empty()) {
      std::cout << func.symbol.c_str() << ".L" << i << std::endl;
      for (const auto& op : block) {
        std::cout << "    " << stringify(op) << std::endl;
      }
    }
  }
}


void PrintProgram(const Program& program) {
  std::cout << std::endl << std::endl;
  std::cout << "-- program" << std::endl;
  for (const auto& [sym, func] : program.functions) {
    std::cout << sym.c_str() << std::endl;
    PrintFunction(func);
  }
}

static i64 GetNodeID(const Function& func, block_label_t block_idx) {
  return (std::uintptr_t)(&func.blocks.at(block_idx));
}

static void VisualizeFunctionHelper(epi::cycle::VisualGraph& graph, const Function& func) {
  for (const auto& [block_idx, block] : func.blocks) {
    const auto id = GetNodeID(func, block_idx);
    graph.n(id).title(cotyl::Format("L%d", block_idx));
    for (const auto& directive : block) {
      directive.visit<void>(
        [&](const Select& select) {
          auto node = graph.n(id, stringify(select));
          for (auto [val, dest] : *select.table) {
            node->n(GetNodeID(func, dest), std::to_string(val));
          }
          if (select._default) {
            node->n(GetNodeID(func, select._default), "default");
          }
        },
        [&](const UnconditionalBranch& branch) {
          auto node = graph.n(id, stringify(branch));
          node->n(GetNodeID(func, branch.dest));
        },
        [&]<typename T>(const BranchCompare<T>& branch) {
          auto node = graph.n(id, stringify(branch));
          node->n(GetNodeID(func, branch.tdest));
          node->n(GetNodeID(func, branch.fdest));
        },
        [&](const auto& dir) {
          auto node = graph.n(id, stringify(dir));
        }
      );
    }
  }

  auto fnode = graph.n((std::uintptr_t)&func);
  fnode.title(func.symbol.str() + "(*)");
  fnode->n(GetNodeID(func, Function::Entry));
  for (const auto& [loc_idx, local] : func.locals) {
    std::string label;
    if (local.arg_idx.has_value()) {
      label = cotyl::Format("%s c%d <- a%d", detail::TypeString(local.type).c_str(), loc_idx, local.arg_idx.value());
    }
    else {
      label = cotyl::Format("%s c%d", detail::TypeString(local.type).c_str(), loc_idx);
    }
    graph.n((std::uintptr_t)&func, label);
  }
}

void VisualizeFunction(const Function& func, const std::string& filename) {
  auto graph = std::make_unique<epi::cycle::VisualGraph>();

  VisualizeFunctionHelper(*graph, func);

  graph->allow_multi_edge = true;
  graph->Visualize(filename);
}
 
void VisualizeProgram(const Program& program, const std::string& filename) {
  auto graph = std::make_unique<epi::cycle::VisualGraph>();

  for (const auto& [symbol, func] : program.functions) {
    VisualizeFunctionHelper(*graph, func);
  }

  graph->allow_multi_edge = true;
  graph->Visualize(filename);
}

STRINGIFY_METHOD(Pointer) { 
  return cotyl::Format("%p", value.value); 
}

STRINGIFY_METHOD(Struct) { 
  return "<struct type>"; 
}

STRINGIFY_METHOD(Global) {
  return swl::visit<std::string>(
    swl::overloaded{
      [](const epi::calyx::Pointer& glob) {
        return cotyl::Format("%%p%016x", glob.value);
      },
      [](const epi::calyx::LabelOffset& glob) {
        if (glob.offset) {
          return cotyl::Format("&%s + %llx", glob.label.c_str(), glob.offset);
        }
        else {
          return cotyl::Format("&%s", glob.label.c_str());
        }
      },
      []<typename T>(const epi::calyx::Scalar<T>& glob) {
        return std::to_string(+glob.value);
      },
      // exhaustive variant access
      [](const auto& invalid) { static_assert(!sizeof(invalid)); }
    }, 
    value
  );
}

}