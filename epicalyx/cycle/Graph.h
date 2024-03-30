#pragma once 

#include "Containers.h"


namespace epi {

template<typename I, typename T>
struct Graph {

  struct Node {
    T* value;
    cotyl::flat_set<I> to{};
    cotyl::flat_set<I> from{};
  };

private:
  cotyl::unordered_map<I, Node> nodes{};

public:
  auto begin() { return nodes.begin(); }
  auto end() { return nodes.end(); }
  auto begin() const { return nodes.begin(); }
  auto end() const { return nodes.end(); }

  Node& operator[](I idx) {
    return nodes.at(idx);
  }

  const Node& At(I idx) const {
    return nodes.at(idx);
  }

  void Erase(I idx) {
    nodes.erase(idx);
  }

  void Clear(I idx) {
    auto& node = nodes.at(idx);
    node.to = {};
    node.from = {};
  }

  void AddNode(I idx, T* value) {
    nodes.emplace(idx, Node{value});
  }

  void AddNodeIfNotExists(I idx, T* value) {
    if (!nodes.contains(idx)) {
      nodes.emplace_hint(nodes.end(), idx, Node{value});
    }
  }

  void AddEdge(I from, I to) {
    nodes.at(from).to.emplace(to);
    nodes.at(to).from.emplace(from);
  }

  void RemoveEdge(I from, I to) {
    nodes.at(from).to.erase(to);
    nodes.at(to).from.erase(from);
  }

  std::vector<I> TopSort() const;
  std::vector<std::vector<I>> LayeredTopSort() const;

  // find common ancestor for 2 nodes such that all paths to these nodes go through that ancestor
  I CommonAncestor(I first, I second) const;
  std::vector<I> OrderedUpwardClosure(I base) const;
  cotyl::unordered_set<I> UpwardClosure(cotyl::unordered_set<I>&& base) const;
  bool IsAncestorOf(I base, I other) const;
};

template<typename I, typename T>
std::vector<I> Graph<I, T>::TopSort() const {
  std::vector<I> result{};
  cotyl::unordered_set<I> todo{};


  for (const auto &[idx, node] : nodes) {
    // nodes that have no inputs
    if (node.from.empty()) {
      result.push_back(idx);
    }
    else {
      todo.insert(idx);
    }
  }

  while (!todo.empty()) {
    auto order_size = result.size();
    for (const auto& id : todo) {
      // check if all inputs are done
      const auto& node = At(id);
      bool allow_add = true;
      for (const auto& from_id : node.from) {
        if (todo.contains(from_id)) {
          allow_add = false;
          break;
        }
      }

      if (allow_add) {
        result.push_back(id);
      }
    }

    // for cycles, pick the node with the least inputs left
    if (order_size == result.size()) {
      u32 least_inputs = -1;
      I least_id = 0;
      for (const auto& id : todo) {
        const auto& node = At(id);
        u32 inputs = 0;
        for (const auto& from_id : node.from) {
          if (todo.contains(from_id)) {
            inputs++;
          }
        }

        if (inputs < least_inputs) {
          least_inputs = inputs;
          least_id = id;
        }
        else if (inputs == least_inputs) {
          least_id = std::min(least_id, id);
        }
      }

      result.push_back(least_id);
    }

    for (auto i = order_size; i < result.size(); i++) {
      todo.erase(result[i]);
    }
  }

  return std::move(result);
}

template<typename I, typename T>
std::vector<std::vector<I>> Graph<I, T>::LayeredTopSort() const {
  std::vector<std::vector<I>> result{};
  cotyl::unordered_set<I> todo{};

  result.push_back({});
  for (const auto &[id, node] : nodes) {
    // first layer is only nodes that have no inputs
    if (node.from.empty()) {
      result.back().push_back(id);
    }
    else {
      todo.insert(id);
    }
  }

  if (result.back().empty()) [[unlikely]] {
    // possible if only loops exist in fist layer
    result.pop_back();
  }

  while (!todo.empty()) {
    result.push_back({});
    for (const auto& id : todo) {
      // check if all inputs are done
      const auto& node = At(id);
      bool allow_add = true;
      for (const auto& from_id : node.from) {
        if (todo.contains(from_id)) {
          allow_add = false;
          break;
        }
      }

      if (allow_add) {
        result.back().push_back(id);
      }
    }

    // for cycles, pick the node with the least inputs left
    if (result.back().empty()) {
      u32 least_inputs = -1;
      u64 least_id = 0;
      for (const auto& id : todo) {
        const auto& node = At(id);
        u32 inputs = 0;
        for (const auto& from_id : node.from) {
          if (todo.contains(from_id)) {
            inputs++;
          }
        }

        if (inputs < least_inputs) {
          least_inputs = inputs;
          least_id = id;
        }
        else if (inputs == least_inputs) {
          least_id = std::min(least_id, id);
        }
      }

      result.back().push_back(least_id);
    }

    for (const auto& id : result.back()) {
      todo.erase(id);
    }
  }

  return std::move(result);
}

template<typename I, typename T>
I Graph<I, T>::CommonAncestor(I first, I second) const {
  cotyl::set<I> ancestors{first, second};

  // we use the fact that in general block1 > block2 then block1 can never be an ancestor of block2
  // it may happen for loops, but then the loop entry is the minimum block, so we want to go there
  cotyl::unordered_set<I> ancestors_considered{};

  while (ancestors.size() > 1) {
    auto max_ancestor = *ancestors.rbegin();
    ancestors.erase(std::prev(ancestors.end()));
    ancestors_considered.emplace(max_ancestor);
    if (!nodes.contains(max_ancestor)) [[unlikely]] {
      return 0;
    }

    auto& deps = nodes.at(max_ancestor);
    if (deps.from.empty()) [[unlikely]] {
      return 0;
    }
    for (auto dep : deps.from) {
      if (dep < max_ancestor || !ancestors_considered.contains(dep)) {
        ancestors.insert(dep);
      }
    }
  }

  // at this point only one ancestor should be left
  return *ancestors.begin();
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
    if (node != nodes.end()) {
      for (const auto& idx : node->second.to) {
        if (!closure_found.contains(idx)) {
          closure_found.emplace_hint(idx, closure_found.end());
          search.emplace(idx);
          closure.push_back(idx);
        }
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
    if (node != nodes.end()) {
      for (const auto& idx : node->second.to) {
        if (!closure.contains(idx)) {
          closure.emplace_hint(closure.end(), idx);
          search.emplace(idx);
        }
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
    if (node != nodes.end()) {
      for (const auto& idx : node->second.to) {
        if (idx == other) {
          return true;
        }

        if (!closure_found.contains(idx)) {
          closure_found.emplace_hint(closure_found.end(), idx);
          search.emplace(idx);
        }
      }
    }
  }

  return false;
}

}