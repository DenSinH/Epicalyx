#include "Cycle.h"

#include "Format.h"
#include "Containers.h"
#include "SStream.h"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>


namespace epi::cycle {

void VisualGraph::Visualize(const std::string& filename) {
  Agdesc_t desc = {
    .directed = directed,            /* if edges are asymmetric */
    .strict = !allow_multi_edge,     /* if multi-edges forbidden */
    .no_loop = acyclic,              /* if no loops */
    .maingraph = 1,                  /* if this is the top level graph */
    .no_write = 0,                   /* if a temporary subgraph */
    .has_attrs = 0,                  /* if string attr tables should be initialized */
    .has_cmpnd = 0,                  /* if may contain collapsed nodes */
  };
  
  GVC_t *gvc = gvContext();
  Agraph_t *g = agopen((char*)"g", desc, 0);

  if (square_nodes) agattr(g, AGNODE, (char*)"shape", (char*)"rectangle");
  else agattr(g, AGNODE, (char*)"shape", (char*)"circle");
  agattr(g, AGNODE, (char*)"fontname", (char*)"Courier-Bold");
  agattr(g, AGNODE, (char*)"fontsize", (char*)"10");

  for (auto& [_, node] : graph) {
    auto& vnode = node.value;
    auto* n = vnode.agnode = agnode(g, const_cast<char*>(std::to_string(vnode.id).c_str()), 1);
    cotyl::StringStream label{};
    if (!vnode.title.empty()) {
      label << vnode.title;
    }
    else {
      label << std::to_string(vnode.id);
    }

    if (!vnode.body.empty()) {
      label << "\\l";
      for (const auto& line : vnode.body) {
        label << "\\l" << line;
      }
      label << "\\l";
    }
    
    agset(n, (char*)"label", const_cast<char*>(label.finalize().c_str()));
  }

  for (auto& [_, node] : graph) {
    for (const auto& to_idx : node.to) {
      auto& to_node = graph[to_idx];
      if (node.value.outputs.contains(to_idx)) {
        const auto& label = node.value.outputs.at(to_idx);
        agedge(g, (Agnode_t*)node.value.agnode, (Agnode_t*)to_node.value.agnode, const_cast<char*>(label.c_str()), true);
      }
      else {
        agedge(g, (Agnode_t*)node.value.agnode, (Agnode_t*)to_node.value.agnode, nullptr, true);
      }
    }
  }
  
  gvLayout(gvc, g, "dot");
  auto error = gvRenderFilename(gvc, g, "pdf", filename.c_str());
  if (error) {
    throw cotyl::FormatExcept("Graphviz error %d", error);
  }

  gvFreeLayout(gvc, g);
  agclose(g);
}

}