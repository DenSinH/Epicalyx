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

  VisualGraph() : graph{} { }
  template<typename I, typename T, typename TFunc, typename BFunc>
  VisualGraph(const Graph<I, T>& graph, TFunc title, BFunc body);

  enum class NodeSort {
    Topological,
    Circle
  };

  void Visualize(NodeSort sort);
  void Join();

  struct VisualNode {
    VisualNode(u64 id) : id(id) { }
    VisualNode(u64 id, const std::string& title, std::vector<std::string>&& body) : 
        id(id), title{title}, body{std::move(body)} { }

    u64 id;
    bool selected = false;
    std::string title{};
    std::vector<std::string> body{};
    cotyl::unordered_map<u64, std::string> outputs{};
    cotyl::unordered_set<std::string> output_set{};
  };

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

  Graph<u64, VisualNode> graph{};
  std::thread thread;

  void* window;
  int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;
  void* gl_context;

  void InitSDL();
  void InitImGui();
  top_sort_t FindOrder();
  void VisualizeImpl(NodeSort sort);
  void Render(ImNodes::CanvasState& canvas, cotyl::unordered_map<u64, ImVec2>& positions);

public:
  NodeRef n(u64 from, std::string line = "") {
    auto& node = graph.EmplaceNodeIfNotExists(from, from);
    if (!line.empty()) {
      node.value.body.push_back(line);
    }
    return NodeRef(*this, from);
  }
};

template<typename I, typename T, typename TFunc, typename BFunc>
VisualGraph::VisualGraph(const Graph<I, T>& g, TFunc title, BFunc body) {
  for (const auto& [node_idx, node] : g) {
    graph.AddNode(node_idx, VisualNode{
      static_cast<u64>(node_idx),
      title(node_idx, node.value),
      body(node_idx, node.value)
    });
  }

  for (const auto& [node_idx, node] : g) {
    for (const auto& to_idx : node.to) {
      graph.AddEdge(node_idx, to_idx);
      auto& vnode = graph[node_idx].value;
      vnode.outputs.emplace(to_idx, "");
      vnode.output_set.emplace("");
    }
  }
}

}
