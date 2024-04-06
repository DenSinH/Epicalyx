#pragma once 

#include "Default.h"
#include "Containers.h"
#include "CustomAssert.h"

#include <numeric>
#include <algorithm>


namespace epi {

template<typename I, typename T, bool Directed>
struct Graph {
  // dummy because nested struct template specialization
  // has to happen AFTER main struct specialization...
  template<bool D, typename dummy = void> struct Node;

  template<typename dummy>
  struct Node<true, dummy> {
    T value;
    cotyl::unordered_set<I> to{};
    cotyl::unordered_set<I> from{};
  };

  template<typename dummy>
  struct Node<false, dummy> {
    T value;
    cotyl::unordered_set<I> to{};
  };

  using node_t = Node<Directed>;

private:
  cotyl::unordered_map<I, node_t> nodes{};

public:
  std::size_t size() const { return nodes.size(); }
  auto begin() { return nodes.begin(); }
  auto end() { return nodes.end(); }
  auto begin() const { return nodes.begin(); }
  auto end() const { return nodes.end(); }

  bool Has(I idx) const {
    return nodes.contains(idx);
  }

  node_t& operator[](I idx) {
    return nodes.at(idx);
  }

  const node_t& At(I idx) const {
    return nodes.at(idx);
  }

  void Erase(I idx) {
    nodes.erase(idx);
  }

  void Reserve(size_t n) {
    nodes.reserve(n);
  }

  void ClearNode(I idx) {
    auto& node = nodes.at(idx);
    node.to.clear();
    if constexpr(Directed) {
      node.from.clear();
    }
  }

  void Clear() {
    nodes.clear();
  }

  template<typename... Args>
  node_t& AddOrAssignNode(I idx, Args... args) {
    return nodes.insert_or_assign(idx, node_t{T(args...)}).first->second;
  }

  node_t& AddOrAssignNode(I idx, T&& value) {
    return nodes.insert_or_assign(idx, node_t{std::move(value)}).first->second;
  }

  node_t& AddOrAssignNode(I idx, const T& value) {
    return nodes.insert_or_assign(idx, node_t{value}).first->second;
  }

  template<typename... Args>
  node_t& EmplaceNodeIfNotExists(I idx, Args... args) {
    return nodes.emplace(idx, node_t{T(args...)}).first->second;
  }

  node_t& AddNodeIfNotExists(I idx, T&& value) {
    return nodes.emplace(idx, node_t{std::move(value)}).first->second;
  }

  node_t& AddNodeIfNotExists(I idx, const T& value) {
    return nodes.emplace(idx, node_t{value}).first->second;
  }

  void AddEdge(I from, I to) {
    nodes.at(from).to.emplace(to);
    if constexpr(Directed) {
      nodes.at(to).from.emplace(from);
    }
    else {
      nodes.at(to).to.emplace(from);
    }
  }

  void RemoveEdge(I from, I to) {
    nodes.at(from).to.erase(to);
    if constexpr(Directed) {
      nodes.at(to).from.erase(from);
    }
    else {
      nodes.at(to).to.erase(from);
    }
  }

  cotyl::unordered_set<I> UpwardCluster(cotyl::unordered_set<I>&& base) const;
  bool ExistsPath(I base, I other) const;
};

// topsort only makes sense for directed graphs
template<typename I, typename T>
std::vector<I> TopSort(const Graph<I, T, true>& graph, bool acyclic) {
  std::vector<I> result{};
  result.reserve(graph.size());
  cotyl::unordered_set<I> todo{};
  cotyl::flat_set<I> candidates{};

  for (const auto& [idx, node] : graph) {
    // nodes that have no inputs
    if (node.from.empty()) {
      result.push_back(idx);
      for (const auto& to_idx : node.to) {
        candidates.emplace(to_idx);
      }
    }
    else {
      todo.emplace(idx);
    }
  }

  if (!acyclic) {
    // this can happen if there is a loop at the first iteration
    // just insert some id
    if (candidates.empty() && !todo.empty()) [[unlikely]] {
      candidates.emplace(*todo.begin());
    }
  }

  while (!candidates.empty()) {
    auto order_size = result.size();
    // all candidates MUST be in todo,
    // as they are selected as the nodes with an input from the previous "layer"
    // these CANNOT have been added to the result yet, nor any any "non-candidates"
    // be added to the result in this iteration, as they would have been added before
    // (some node was preventing them from being added)
    for (const auto& id : candidates) {
      // check if all inputs are done
      const auto& node = graph.At(id);
      bool allow_add = std::none_of(
        node.from.begin(), node.from.end(), [&](const auto& from_id) { return todo.contains(from_id); }
      );
      if (allow_add) {
        result.push_back(id);
      }
    }

    // for cycles, pick the node with the least inputs left
    if (order_size == result.size()) {
      std::pair<u32, I> least = {-1, I{}};
      for (const auto& id : candidates) {
        const auto& node = graph.At(id);
        auto inputs = std::count_if(
          node.from.begin(), node.from.end(), [&](const auto& from_id) { return todo.contains(from_id); }
        );
        least = std::min(least, {inputs, id});
      }

      result.push_back(least.second);
    }

    candidates.clear();
    for (auto i = order_size; i < result.size(); i++) {
      todo.erase(result[i]);
      for (const auto& to_idx : graph.At(result[i]).to) {
        candidates.emplace(to_idx);
      }
    }

    if (!acyclic) {
      cotyl::erase_if_set(candidates, [&](const auto& idx) { return !todo.contains(idx); });
    }
  }

  cotyl::Assert(result.size() == graph.size(), "Not all nodes added to topological sort!");
  return std::move(result);
}

template<typename I, typename T>
std::vector<std::vector<I>> LayeredTopSort(const Graph<I, T, true>& graph, bool acyclic) {
  std::vector<std::vector<I>> result{};
  cotyl::unordered_set<I> todo{};
  cotyl::flat_set<I> candidates{};

  result.push_back({});
  for (const auto& [id, node] : graph) {
    // first layer is only nodes that have no inputs
    if (node.from.empty()) {
      result.back().push_back(id);
      for (const auto& to_idx : node.to) {
        candidates.emplace(to_idx);
      }
    }
    else {
      todo.insert(id);
    }
  }

  if (!acyclic) {
    if (result.back().empty() && !todo.empty()) [[unlikely]] {
      // possible if only loops exist in fist layer
      result.pop_back();
      candidates.emplace(*todo.begin());
    }
  }

  while (!candidates.empty()) {
    // see remark before on candidates always being filled
    result.push_back({});
    for (const auto& id : candidates) {
      // check if all inputs are done
      const auto& node = graph.At(id);
      bool allow_add = std::none_of(
        node.from.begin(), node.from.end(), [&](const auto& from_id) { return todo.contains(from_id); }
      );

      if (allow_add) {
        result.back().push_back(id);
      }
    }

    // for cycles, pick the node with the least inputs left
    if (result.back().empty()) {
      std::pair<u32, I> least = {-1, I{}};
      for (const auto& id : candidates) {
        const auto& node = graph.At(id);
        auto inputs = std::count_if(
          node.from.begin(), node.from.end(), [&](const auto& from_id) { return todo.contains(from_id); }
        );
        least = std::min(least, {inputs, id});
      }

      result.back().push_back(least.second);
    }

    candidates.clear();
    for (const auto& id : result.back()) {
      todo.erase(id);
      for (const auto& to_idx : graph.At(id).to) {
        candidates.emplace(to_idx);
      }
    }

    if (!acyclic) {
      // remove any candidates that were done already
      cotyl::erase_if_set(candidates, [&](const auto& idx) { return !todo.contains(idx); });
    }
  }

  cotyl::Assert(
    std::accumulate(
      result.begin(),
      result.end(),
      0,
      [](auto acc, const auto& layer) { return acc + layer.size(); }
    ) == graph.size(), 
    "Not all nodes added to topological sort!"
  );
  return std::move(result);
}

// "ordered" upward closure only makes sense for directed graphs
template<typename I, typename T>
std::vector<I> OrderedUpwardClosure(const Graph<I, T, true>& graph, I base) {
  cotyl::unordered_set<I> closure_found{base};
  std::vector<I> closure{};
  closure.push_back(base);
  cotyl::unordered_set<I> search{base};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    const auto& node = graph.At(current);
    for (const auto& idx : node.to) {
      if (closure_found.emplace(idx).second) {
        search.emplace(idx);
        closure.push_back(idx);
      }
    }
  }

  return closure;
}

template<typename I, typename T, bool Directed>
cotyl::unordered_set<I> Graph<I, T, Directed>::UpwardCluster(cotyl::unordered_set<I>&& base) const {
  cotyl::unordered_set<I> closure = std::move(base);
  cotyl::unordered_set<I> search = {closure.begin(), closure.end()};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    const auto& node = nodes.at(current);
    for (const auto& idx : node.to) {
      if (!closure.contains(idx)) {
        closure.emplace_hint(closure.end(), idx);
        search.emplace(idx);
      }
    }
  }

  return closure;
}

template<typename I, typename T, bool Directed>
bool Graph<I, T, Directed>::ExistsPath(I base, I other) const {
  if (base == other) return true;
  cotyl::unordered_set<I> closure_found{base};
  cotyl::unordered_set<I> search{base};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    const auto& node = nodes.at(current);
    for (const auto& idx : node.to) {
      if (idx == other) {
        return true;
      }

      if (closure_found.emplace(idx).second) {
        search.emplace(idx);
      }
    }
  }

  return false;
}

}