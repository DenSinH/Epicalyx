#pragma once

#include "Default.h"
#include "Containers.h"
#include "Graph.h"

#include <thread>
#include <vector>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720


namespace ImNodes {
struct CanvasState;
}

struct ImVec2;

namespace epi::cycle {

struct VisualGraph {

  void Visualize();
  void Join();

private:
  using top_sort_t = std::vector<std::vector<u64>>;

  struct NodeRef {
    NodeRef(VisualGraph& graph, u64 from) :
        graph(graph), from(from) {
      graph.graph.EmplaceNodeIfNotExists(from, from);
    }

    VisualGraph& graph;
    u64 from;

    NodeRef* operator->() {
      return this;
    }

    NodeRef n(u64 to, const std::string& output = "") {
      auto ref = NodeRef(graph, to);  // instantiates to node if needed
      graph.graph.AddEdge(from, to);
      auto& vnode = graph.graph[from].value;
      vnode.outputs.emplace(to, output);
      vnode.output_set.emplace(output);
      return ref;
    }

    void title(const std::string& title) {
      graph.graph[from].value.title = title;
    }
  };

  struct VisualNode {
    VisualNode(u64 id) : id(id) { }

    u64 id;
    bool selected = false;
    std::string title;
    std::vector<std::string> body{};
    cotyl::unordered_map<u64, std::string> outputs{};
    cotyl::unordered_set<std::string> output_set{};
  };

  Graph<u64, VisualNode> graph{};
  std::thread thread;

  void* window;
  int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;
  void* gl_context;

  void InitSDL();
  void InitImGui();
  top_sort_t FindOrder();
  void VisualizeImpl();
  void Render(ImNodes::CanvasState& canvas, const top_sort_t& sort, cotyl::unordered_map<u64, ImVec2>& positions);

public:
  NodeRef n(u64 from, std::string line = "") {
    auto& node = graph.EmplaceNodeIfNotExists(from, from);
    if (!line.empty()) {
      node.value.body.push_back(line);
    }
    return NodeRef(*this, from);
  }
};

}
