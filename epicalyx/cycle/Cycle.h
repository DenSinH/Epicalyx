#pragma once

#include "Default.h"
#include "imgui/imgui.h"

#include <thread>
#include <map>
#include <vector>
#include <set>

namespace ImNodes {
struct CanvasState;
}

namespace epi::cycle {

struct Graph {

  void Visualize();
  void Join();

private:
  using top_sort_t = std::vector<std::vector<u64>>;

  struct NodeRef {
    NodeRef(Graph& graph, u64 from) :
        graph(graph), from(from) {
      if (!graph.nodes.contains(from)) {
        graph.nodes.emplace(from, Node(from));
      }
    }

    Graph& graph;
    u64 from;

    NodeRef* operator->() {
      return this;
    }

    NodeRef n(u64 to, const std::string& output = "") {
      if (!graph.edges.contains(to)) {
        graph.edges.insert({to, {}});
      }
      graph.edges[to].emplace_back(from, output);
      graph.nodes.at(from).outputs.emplace(output);
      return NodeRef(graph, to);
    }
  };

  struct Node {
    Node(u64 id) : id(id) { }

    u64 id;
    bool selected = false;
    ImVec2 pos{0, 0};
    std::vector<std::string> body{};
    std::set<std::string> outputs{};
  };

  struct Edge {
    Edge(u64 from, const std::string& output) : from(from), output(output) { }

    u64 from;
    std::string output;
  };

  std::map<u64, Node> nodes{};
  std::map<u64, std::vector<Edge>> edges{};  // { to: Edge }

  std::thread thread;

  void* window;
  void* gl_context;

  void InitSDL();
  void InitImGui();
  top_sort_t FindOrder();
  void VisualizeImpl();
  void Render(ImNodes::CanvasState& canvas, const top_sort_t& sort);

public:
  NodeRef n(u64 from, std::string line = "") {
    if (!line.empty()) {
      if (!nodes.contains(from)) {
        nodes.insert({from, Node(from)});
      }
      nodes.at(from).body.push_back(line);
    }
    return NodeRef(*this, from);
  }
};

}
