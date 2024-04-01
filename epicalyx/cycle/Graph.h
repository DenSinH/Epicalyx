#pragma once 

#include "Default.h"
#include "Containers.h"
#include "CustomAssert.h"

#include <numeric>
#include <algorithm>


namespace epi {

template<typename I, typename T>
struct Graph {

  struct Node {
    T value;
    cotyl::flat_set<I> to{};
    cotyl::flat_set<I> from{};
  };

private:
  cotyl::unordered_map<I, Node> nodes{};

public:
  std::size_t size() const { return nodes.size(); }
  auto begin() { return nodes.begin(); }
  auto end() { return nodes.end(); }
  auto begin() const { return nodes.begin(); }
  auto end() const { return nodes.end(); }

  bool Has(I idx) const {
    return nodes.contains(idx);
  }

  Node& operator[](I idx) {
    return nodes.at(idx);
  }

  const Node& At(I idx) const {
    return nodes.at(idx);
  }

  void Erase(I idx) {
    nodes.erase(idx);
  }

  void Reserve(size_t n) {
    nodes.reserve(n);
  }

  void Clear(I idx) {
    auto& node = nodes.at(idx);
    node.to.clear();
    node.from.clear();
  }

  template<typename... Args>
  Node& EmplaceNode(I idx, Args... args) {
    return nodes.emplace(idx, Node{T(args...)}).first->second;
  }

  Node& AddNode(I idx, T&& value) {
    return nodes.emplace(idx, Node{std::move(value)}).first->second;
  }

  Node& AddNode(I idx, const T& value) {
    return nodes.emplace(idx, Node{value}).first->second;
  }

  template<typename... Args>
  Node& EmplaceNodeIfNotExists(I idx, Args... args) {
    auto node = nodes.find(idx);
    if (node == nodes.end()) {
      return nodes.emplace_hint(node, idx, Node{T(args...)})->second;
    }
    return node->second;
  }

  Node& AddNodeIfNotExists(I idx, T&& value) {
    auto node = nodes.find(idx);
    if (node == nodes.end()) {
      return nodes.emplace_hint(nodes.end(), idx, Node{std::move(value)})->second;
    }
    return node->second;
  }

  void AddEdge(I from, I to) {
    nodes.at(from).to.emplace(to);
    nodes.at(to).from.emplace(from);
  }

  void AddDoubleEdge(I n1, I n2) {
    auto& node1 = nodes.at(n1);
    auto& node2 = nodes.at(n2);
    node1.from.emplace(n2);
    node1.to.emplace(n2);
    node2.from.emplace(n1);
    node2.to.emplace(n1);
  }

  void RemoveEdge(I from, I to) {
    nodes.at(from).to.erase(to);
    nodes.at(to).from.erase(from);
  }

  template<bool Acyclic>
  std::vector<I> TopSort() const;
  template<bool Acyclic>
  std::vector<std::vector<I>> LayeredTopSort() const;

  std::vector<I> OrderedUpwardClosure(I base) const;
  cotyl::unordered_set<I> UpwardClosure(cotyl::unordered_set<I>&& base) const;
  bool IsAncestorOf(I base, I other) const;
};

template<typename I, typename T>
template<bool Acyclic>
std::vector<I> Graph<I, T>::TopSort() const {
  std::vector<I> result{};
  cotyl::unordered_set<I> todo{};
  cotyl::flat_set<I> candidates{};

  for (const auto& [idx, node] : nodes) {
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

  if constexpr(!Acyclic) {
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
      const auto& node = At(id);
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
        const auto& node = At(id);
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
      for (const auto& to_idx : nodes.at(result[i]).to) {
        candidates.emplace(to_idx);
      }
    }

    if constexpr (!Acyclic) {
      cotyl::erase_if_set(candidates, [&](const auto& idx) { return !todo.contains(idx); });
    }
  }

  cotyl::Assert(result.size() == nodes.size(), "Not all nodes added to topological sort!");
  return std::move(result);
}

template<typename I, typename T>
template<bool Acyclic>
std::vector<std::vector<I>> Graph<I, T>::LayeredTopSort() const {
  std::vector<std::vector<I>> result{};
  cotyl::unordered_set<I> todo{};
  cotyl::flat_set<I> candidates{};

  result.push_back({});
  for (const auto& [id, node] : nodes) {
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

  if constexpr(!Acyclic) {
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
      const auto& node = At(id);
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
        const auto& node = At(id);
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
      for (const auto& to_idx : nodes.at(id).to) {
        candidates.emplace(to_idx);
      }
    }

    if constexpr (!Acyclic) {
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
    ) == nodes.size(), 
    "Not all nodes added to topological sort!"
  );
  return std::move(result);
}

template<typename I, typename T>
std::vector<I> Graph<I, T>::OrderedUpwardClosure(I base) const {
  cotyl::unordered_set<I> closure_found{base};
  std::vector<I> closure{};
  closure.push_back(base);
  cotyl::unordered_set<I> search{base};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    auto node = nodes.find(current);
    for (const auto& idx : node->second.to) {
      if (closure_found.emplace(idx).second) {
        search.emplace(idx);
        closure.push_back(idx);
      }
    }
  }

  return closure;
}

template<typename I, typename T>
cotyl::unordered_set<I> Graph<I, T>::UpwardClosure(cotyl::unordered_set<I>&& base) const {
  cotyl::unordered_set<I> closure = std::move(base);
  cotyl::unordered_set<I> search = {closure.begin(), closure.end()};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    auto node = nodes.find(current);
    for (const auto& idx : node->second.to) {
      if (!closure.contains(idx)) {
        closure.emplace_hint(closure.end(), idx);
        search.emplace(idx);
      }
    }
  }

  return closure;
}

template<typename I, typename T>
bool Graph<I, T>::IsAncestorOf(I base, I other) const {
  cotyl::unordered_set<I> closure_found{base};
  cotyl::unordered_set<I> search{base};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    auto node = nodes.find(current);
    for (const auto& idx : node->second.to) {
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