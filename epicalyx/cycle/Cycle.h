#pragma once

#include "Default.h"
#include "Containers.h"
#include "Graph.h"

#include "Vector.h"


namespace epi::cycle {

struct VisualGraph {

  VisualGraph() : graph{} { }
  template<typename I, typename T, bool Directed, typename TFunc, typename BFunc>
  VisualGraph(const Graph<I, T, Directed>& graph, TFunc title, BFunc body);

  bool directed = true;
  bool acyclic = false;
  bool allow_multi_edge = false;
  bool square_nodes = true;

  void Visualize(const std::string& filename);

private:

  struct VisualNode {
    VisualNode(u64 id) : id(id) { }
    VisualNode(u64 id, const std::string& title, cotyl::vector<std::string>&& body) : 
        id(id), title{title}, body{std::move(body)} { }

    u64 id;
    void* agnode = nullptr;
    bool selected = false;
    std::string title{};
    cotyl::vector<std::string> body{};
    cotyl::unordered_map<u64, cotyl::unordered_set<std::string>> outputs{};
  };

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
      vnode.outputs[to].emplace(output);
      return ref;
    }

    void title(const std::string& title) {
      graph.graph[from].value.title = title;
    }
  };

  Graph<u64, VisualNode, true> graph{};

public:
  NodeRef n(u64 from, std::string line = "") {
    auto& node = graph.EmplaceNodeIfNotExists(from, from);
    if (!line.empty()) {
      node.value.body.push_back(line);
    }
    return NodeRef(*this, from);
  }
};

template<typename I, typename T, bool Directed, typename TFunc, typename BFunc>
VisualGraph::VisualGraph(const Graph<I, T, Directed>& g, TFunc title, BFunc body) {
  directed = Directed;
  
  for (const auto& [node_idx, node] : g) {
    graph.AddOrAssignNode(node_idx, VisualNode{
      static_cast<u64>(node_idx),
      title(node_idx, node.value),
      body(node_idx, node.value)
    });
  }

  for (const auto& [node_idx, node] : g) {
    for (const auto& to_idx : node.to) {
      graph.AddEdge(node_idx, to_idx);
      auto& vnode = graph[node_idx].value;
      vnode.outputs[to_idx].emplace("");
    }
  }
}

}
