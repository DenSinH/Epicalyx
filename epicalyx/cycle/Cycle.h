#pragma once

#include "Default.h"
#include "Containers.h"

#include <thread>
#include <vector>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720


namespace ImNodes {
struct CanvasState;
}

struct ImVec2;

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
        graph.edges.emplace(to, std::vector<Edge>{});
      }
      graph.edges[to].emplace_back(from, output);
      graph.nodes.at(from).outputs.emplace(output);
      return NodeRef(graph, to);
    }

    void title(const std::string& title) {
      graph.nodes.at(from).title = title;
    }
  };

  struct Node {
    Node(u64 id) : id(id) { }

    u64 id;
    bool selected = false;
    std::string title;
    std::vector<std::string> body{};
    cotyl::set<std::string> outputs{};
  };

  struct Edge {
    Edge(u64 from, const std::string& output) : from(from), output(output) { }

    u64 from;
    std::string output;
  };

  cotyl::map<u64, Node> nodes{};
  cotyl::map<u64, std::vector<Edge>> edges{};  // { to: Edge }

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
    if (!line.empty()) {
      if (!nodes.contains(from)) {
        nodes.emplace(from, Node(from));
      }
      nodes.at(from).body.push_back(line);
    }
    return NodeRef(*this, from);
  }
};

}
