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
      if (!graph.nodes.contains(from)) {
        graph.nodes.emplace_hint(graph.nodes.end(), from, VisualNode(from));
        graph.graph.AddNode(from, nullptr);
      }
    }

    VisualGraph& graph;
    u64 from;

    NodeRef* operator->() {
      return this;
    }

    NodeRef n(u64 to, const std::string& output = "") {
      auto ref = NodeRef(graph, to);  // instantiates to node if needed
      graph.graph.AddEdge(from, to);
      graph.nodes.at(from).outputs.emplace(to, output);
      graph.nodes.at(from).output_set.emplace(output);
      return ref;
    }

    void title(const std::string& title) {
      graph.nodes.at(from).title = title;
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

  cotyl::unordered_map<u64, VisualNode> nodes{};
  Graph<u64, std::nullptr_t> graph{};  // dynamic allocation makes userdata useless
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
    if (!nodes.contains(from)) {
      nodes.emplace_hint(nodes.end(), from, VisualNode(from));
      graph.AddNode(from, nullptr);
    }
    if (!line.empty()) {
      nodes.at(from).body.push_back(line);
    }
    return NodeRef(*this, from);
  }
};

}
