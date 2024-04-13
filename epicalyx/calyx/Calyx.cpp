#include "Calyx.h"
#include "Directive.h"
#include "cycle/Cycle.h"

#include "SStream.h"
#include "Format.h"
#include "Hash.h"
#include "Cast.h"
#include "Format.h"
#include "Exceptions.h"

#include <iostream>
#include <tuple>


namespace epi::calyx {

namespace detail {

std::string TypeString(const Local::Type& type);

}

size_t Function::Hash() const {
  size_t seed = blocks.size();
  cotyl::map<block_label_t, const block_t&> sorted{blocks.begin(), blocks.end()};
  for (const auto& [block_idx, block] : sorted) {
    calyx::hash_combine(seed, block_idx);
    for (const auto& directive : block) {
      calyx::hash_combine(seed, directive->type_id);
    }
  }
  return seed;
}

size_t Program::Hash() const {
  size_t seed = functions.size();
  cotyl::map<std::string, const Function&> sorted{functions.begin(), functions.end()};
  for (const auto& [sym, function] : functions) {
    calyx::hash_combine(seed, function.Hash());
  }
  return seed;
}


void PrintProgram(const Program& program) {
  std::cout << std::endl << std::endl;
  std::cout << "-- program" << std::endl;
  for (const auto& [sym, func] : program.functions) {
    std::cout << sym << std::endl;
    for (const auto& [i, block] : func.blocks) {
      if (!block.empty()) {
        std::cout << sym << ".L" << i << std::endl;
        for (const auto& op : block) {
          std::cout << "    " << stringify(op) << std::endl;
        }
      }
    }
  }
}

static i64 GetNodeID(const Function& func, block_label_t block_idx) {
  return (std::uintptr_t)(&func.blocks.at(block_idx));
}

void VisualizeProgram(const Program& program, const std::string& filename) {
  auto graph = std::make_unique<epi::cycle::VisualGraph>();

  for (const auto& [symbol, func] : program.functions) {
    for (const auto& [block_idx, block] : func.blocks) {
      const auto id = GetNodeID(func, block_idx);
      graph->n(id).title(cotyl::Format("L%d", block_idx));
      for (const auto& directive : block) {
        switch (directive->cls) {
          case Directive::Class::Expression:
          case Directive::Class::Stack:
          case Directive::Class::Store:
          case Directive::Class::Return:
          case Directive::Class::Call:  // todo
            graph->n(id, stringify(directive));
            break;
          case Directive::Class::Branch: {
            auto node = graph->n(id, stringify(directive));
            auto* branch = reinterpret_cast<const Branch*>(&(*directive));
            const auto destinations = branch->Destinations();
            for (const auto& dest : destinations) {
              node->n(GetNodeID(func, dest));
            }
            break;
          }
          case Directive::Class::Select: {
            auto node = graph->n(id, stringify(directive));
            const auto& select = directive.get<Select>();
            for (auto [val, dest] : select.table) {
              node->n(GetNodeID(func, dest), std::to_string(val));
            }
            if (select._default) {
              node->n(GetNodeID(func, select._default), "default");
            }
            break;
          }
        }
      }
    }

    auto fnode = graph->n((std::uintptr_t)&func);
    fnode.title(symbol + "(*)");
    fnode->n(GetNodeID(func, Function::Entry));
    for (const auto& [loc_idx, local] : func.locals) {
      std::string label;
      if (local.arg_idx.has_value()) {
        label = cotyl::Format("%s c%d <- a%d", detail::TypeString(local.type).c_str(), loc_idx, local.arg_idx.value());
      }
      else {
        label = cotyl::Format("%s c%d", detail::TypeString(local.type).c_str(), loc_idx);
      }
      graph->n((std::uintptr_t)&func, label);
    }
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

}