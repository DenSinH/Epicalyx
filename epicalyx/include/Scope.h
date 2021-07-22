#pragma once

#include <map>
#include <vector>
#include <stdexcept>

namespace epi::cotyl {

template<typename K, typename V>
struct Scope {

  void NewLayer() {
    scope.emplace_back();
  }

  void PopLayer() {
    scope.pop_back();
  }

  void Set(const K& key, const V& value) {
    scope.back()[key] = value;
  }

  bool Has(const K& key) const {
    for (auto& s = scope.rbegin(); s != scope.rend(); s++) {
      if (s->contains(key)) {
        return true;
      }
    }
    return false;
  }

  V Get(const K& key) const {
    for (auto& s = scope.rbegin(); s != scope.rend(); s++) {
      if (s->contains(key)) {
        return s->at(key);
      }
    }
    throw std::runtime_error("Invalid scope access");
  }

private:
  std::vector<std::map<K, V>> scope{};
};


}