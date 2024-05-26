#include "Cycle.h"
#include "Exceptions.h"  // for Exception
#include "Format.h"      // for Format

#ifndef NO_GRAPHVIZ
#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>
#endif


namespace epi::cycle {

struct GraphvizError : cotyl::Exception {
  GraphvizError(std::string&& message) : 
      Exception("Graphviz Error", std::move(message)) { }

  GraphvizError(int error) : 
      GraphvizError(cotyl::Format("Error code %d", error)) { }
};

void VisualGraph::Visualize(const std::string& filename) {
#ifndef NO_GRAPHVIZ
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
  agattr(g, AGNODE, (char*)"fontname", (char*)"Courier New");
  agattr(g, AGNODE, (char*)"fontsize", (char*)"10");
  agattr(g, AGEDGE, (char*)"fontname", (char*)"Courier New");
  agattr(g, AGEDGE, (char*)"fontsize", (char*)"10");

  for (auto& [_, node] : graph) {
    auto& vnode = node.value;
    auto* n = vnode.agnode = agnode(g, const_cast<char*>(std::to_string(vnode.id).c_str()), 1);
    cotyl::StringStream label{};
    label << "<b>";
    if (!vnode.title.empty()) {
      label << vnode.title;
    }
    else {
      label << std::to_string(vnode.id);
    }
    label << "</b>";

    if (!vnode.body.empty()) {
      for (const auto& line : vnode.body) {
        label << "<br align=\"left\"/>";
        
        for (const auto& c : line) {
          switch (c) {
            case '<': label << "&lt;"; break;
            case '>': label << "&gt;"; break;
            case '&': label << "&amp;"; break;
            default: label << c; break;
          }
        }
      }
      label << "<br align=\"left\"/>";
    }
    agset(n, (char*)"label", const_cast<char*>(agstrdup_html(g, label.finalize().c_str())));
  }

  for (auto& [_, node] : graph) {
    for (const auto& to_idx : node.to) {
      auto& to_node = graph[to_idx];
      Agedge_t* e;
      for (const auto& label : node.value.outputs.at(to_idx)) {
        e = agedge(g, (Agnode_t*)node.value.agnode, (Agnode_t*)to_node.value.agnode, const_cast<char*>(label.c_str()), true);
        agsafeset(e, (char*)"label", const_cast<char*>(label.c_str()), "");
        // agsafeset(e, (char*)"labeldistance", "2.0", "");
      }
    }
  }
  
  gvLayout(gvc, g, "dot");
  auto error = gvRenderFilename(gvc, g, "pdf", filename.c_str());
  if (error) {
    throw GraphvizError(error);
  }

  gvFreeLayout(gvc, g);
  agclose(g);
#else
  throw GraphvizError("Graphviz is not installed");
#endif
}

}