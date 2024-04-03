#include "RIG.h"
#include "RegisterSpace.h"
#include "Format.h"
#include "optimizer/ProgramDependencies.h"
#include "cycle/Cycle.h"

#include <ranges>


namespace epi {

RIG RIG::GenerateRIG(const Program& program) {
  const auto deps = ProgramDependencies::GetDependencies(program);
  RIG rig{};

  struct InstrLiveliness {
    std::optional<GeneralizedVar> def{};
    // may be more than 2 uses in CALL
    std::vector<GeneralizedVar> use{};
  };
  
  struct Liveliness {
    Liveliness(size_t block_len) : single{block_len} { }

    cotyl::unordered_set<GeneralizedVar> def{};
    cotyl::unordered_set<GeneralizedVar> in{};
    cotyl::unordered_set<GeneralizedVar> out{};
    // no need to track "use" on a block level,
    // as we insert all uses into the "in" set right away,
    // and will never reset "in".

    // definitions and uses at an instruction level
    std::vector<InstrLiveliness> single{};
  };
  
  // initialize empty nodes
  cotyl::unordered_map<block_label_t, Liveliness> liveliness{};
  liveliness.reserve(program.blocks.size());
  for (const auto& [block_idx, block] : program.blocks) {
    liveliness.emplace(block_idx, Liveliness{block.size()});
  }

  // add definitions and uses and RIG nodes
  for (const auto& [var_idx, var] : deps.var_graph) {
    const auto gvar = GeneralizedVar::Var(var_idx);
    liveliness.at(var.created.first).def.emplace(gvar);
    liveliness.at(var.created.first).single[var.created.second].def = gvar;
    rig.graph.AddNodeIfNotExists(gvar.NodeUID(), gvar);

    // insert uses
    for (const auto& pos : var.reads) {
      liveliness.at(pos.first).single[pos.second].use.emplace_back(gvar);
      // first iteration, add to "in" if in "use" and not in "def"
      if (pos.first != var.created.first) {
        liveliness.at(pos.first).in.emplace(gvar);
      }
    }

    // forced "out" for program result
    if (var.program_result.first) 
      liveliness.at(var.program_result.first).out.emplace(gvar);
  }

  // same for locals
  for (const auto& [loc_idx, loc] : deps.local_graph) {
    if (!loc_idx) continue;
    const auto gvar = GeneralizedVar::Local(loc_idx);
    liveliness.at(loc.created.first).def.emplace(gvar);
    liveliness.at(loc.created.first).single[loc.created.second].def = gvar;
    rig.graph.AddNodeIfNotExists(gvar.NodeUID(), std::move(gvar));

    for (const auto& pos : loc.reads) {
      liveliness.at(pos.first).single[pos.second].use.emplace_back(gvar);
      if (pos.first != loc.created.first) {
        liveliness.at(pos.first).in.emplace(gvar);
      }
    }

    for (const auto& pos : loc.writes) {
      liveliness.at(pos.first).single[pos.second].def = gvar;
    }
  }
  
  // program graph may not be acyclic
  const auto sort = TopSort(deps.block_graph, false);
  /* Algorithm:
   * This is an adapted version, since most sources seem to assume that 
   * "use" and "def" are ALWAYS disjoint, which is not the case for me.
   * Also, I have a "forced_out" for program results, as these might not
   * be seen as output for blocks when they should.
   * 
   * For every block b:
   *  out <- U_{b' : succ b} in[b']
   *  in  <- (use U out) - def
   * while not stable
   * 
   * Go through topological sort in reverse, because we keep updating out
   * with the in sets of the successors, meaning the variables propagate
   * faster in reverse order.
   * 
   * I think that if a variable is in "out" at some iteration, it will
   * always be in "out", as the "in" sets will only grow. This means
   * that for stability checking, we only need to compare the sizes of
   * the in and out sets to what they were before.
   * 
   * There is also no need to clear any of the maps, ensuring they will
   * only grow.
   * */
  bool stable;
  do {
    stable = true;
    for (const auto block_idx : std::ranges::views::reverse(sort)) {
      auto& l = liveliness.at(block_idx);
      const auto& node = deps.block_graph.At(block_idx);

      for (const auto& to_idx : node.to) {
        const auto& lprime = liveliness.at(to_idx);
        for (const auto& var : lprime.in) {
          if (l.out.emplace(var).second) {
            stable = false;

            // element added to out, add to in if not in def
            // we need to check this because loops may cause
            // variables to be in both "in" and "def"
            if (!l.def.contains(var)) {
              l.in.emplace(var);
            }
          }
        }
      }
    }
  } while (!stable);

  for (auto& [block_idx, l] : liveliness) {
    /* Single instruction block algorithm:
     *  for every block b:
     *    for every gvar v1 in b.def: 
     *      for every gvar v2 in b.out:
     *        add edge (v1, v2) (so also (v2, v1))
     * 
     * The idea is that two variables have overlap if one is defined,
     * and another needs to be alive AFTER that definition. This
     * results in problems for instruction lifetimes WITHIN a single
     * block. Therefore, we also need:
     * - edges between vars that are defined in the same block, which may NOT
     *   be in "b.out", but have overlapping lifetimes
     * - edges between vars that are defined in a block, and variables that 
     *   are in "b.in", which are not in "b.out", but which go "out" after
     *   the new variable's definition.
     * 
     * We fix this by iterating through the block backwards, and updating
     * in / out according to the "writes".
     * */
    for (const auto& single : std::ranges::views::reverse(l.single)) {
      if (single.def.has_value()) {
        // local written / variable defined, this counts as a def
        // add edge to RIG
        const auto& gvar = single.def.value();

        // remove def from out
        l.out.erase(gvar);

        // add edge to any out variable
        for (const auto& out_idx : l.out) {
          rig.graph.AddEdge(gvar.NodeUID(), out_idx.NodeUID());
        }
      }

      // add any used variables back to the "out" set
      // (they need to be output by any previous instructions)
      for (const auto& gvar : single.use) {
        l.out.emplace(gvar);
      }
    }
  }

  return std::move(rig);
}

void RIG::Reduce(const RegisterSpace& regspace) {
  for (const auto& [nid, node] : graph) {
    cotyl::flat_set<i64> to_remove{};
    const auto node_reg_type = regspace.RegisterType(node.value);
    for (const auto& to_idx : node.to) {
      if (node_reg_type != regspace.RegisterType(graph.At(to_idx).value)) {
        to_remove.emplace(to_idx);
      }
    }
    for (const auto& to_idx : to_remove) {
      graph.RemoveEdge(nid, to_idx);
    }
  }
}

void RIG::Visualize(const std::string& filename) const {
  auto vgraph = cycle::VisualGraph(
    graph,
    [](auto idx, auto gvar) -> std::string {
      if (gvar.is_local) {
        return cotyl::Format("c%d", gvar.idx);
      }
      return cotyl::Format("v%d", gvar.idx);
    },
    [](auto idx, auto gvar) -> std::vector<std::string> { return {}; }
  );

  vgraph.square_nodes = false;
  vgraph.acyclic = false;
  vgraph.directed = false;
  vgraph.allow_multi_edge = false;
  vgraph.Visualize(filename);
}

}