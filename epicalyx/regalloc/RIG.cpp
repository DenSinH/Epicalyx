#include "RIG.h"
#include "optimizer/ProgramDependencies.h"

namespace epi {

RIG RIG::GenerateRIG(const Program& program) {
//   auto deps = ProgramDependencies::GetDependencies(program);
//   struct Liveliness {
//     // vector since we will never encounter duplicates anyway
//     std::vector<GeneralizedVar> def{};
//     cotyl::unordered_set<GeneralizedVar> in{};
//     cotyl::unordered_set<GeneralizedVar> out{};
//     cotyl::unordered_set<GeneralizedVar> use{};
//   };
  
//   cotyl::unordered_map<block_label_t, Liveliness> liveliness{};
//   for (const auto& [block_idx, block] : program.blocks) {
//     liveliness.emplace(block_idx, Liveliness{});
//   }

//   for (const auto& [var_idx, var] : deps.var_graph) {
//     liveliness.at(var.created.first).def.push_back({var_idx});

//     // insert uses
//     for (const auto& pos : var.reads) {
//       liveliness.at(pos.first).use.emplace(var_idx);
//     }
//   }
//   for (const auto& [loc_idx, loc] : deps.local_graph) {
//     liveliness.at(loc.created.first).def.push_back({loc_idx, true});

//     for (const auto& pos : loc.reads) {

//     }
//   }
  return {};
}

}