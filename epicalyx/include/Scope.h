#pragma once

#include "Containers.h"
#include "Format.h"

#include "Vector.h"
#include <stdexcept>

namespace epi::cotyl {

template<typename U>
struct Scope {

  const U& Base() { return scope[0]; }
  const U& Top() { return scope.back(); }
  void NewLayer() { scope.emplace_back(); }
  void PopLayer() { scope.pop_back(); }
  size_t Depth() const { return scope.size(); }
  void Reset() { scope = {{}}; }

  template<typename T>
  auto operator<<(const T& callable) {
    NewLayer();
    auto result = callable();
    PopLayer();
    return std::move(result);
  }

protected:
  cotyl::vector<U> scope{{}};
};

template<typename K, typename V, bool allow_multiple_assignment = false>
struct MapScope : public Scope<cotyl::unordered_map<K, V>> {
  using base = Scope<cotyl::unordered_map<K, V>>;

  void Clear() {
    base::scope.clear();
    base::scope.emplace_back();
  }

  void Set(const K& key, const V& value) {
    if constexpr(!allow_multiple_assignment) {
      if (HasTop(key)) {
        throw cotyl::FormatExceptStr("Redefinition of %s", key);
      }
      base::scope.back().emplace(key, value);
    }
    else {
      base::scope.back().insert_or_assign(key, value);
    }
  }
  
  void Set(const K& key, V&& value) {
    if constexpr(!allow_multiple_assignment) {
      if (HasTop(key)) {
        throw cotyl::FormatExceptStr("Redefinition of %s", key);
      }
      base::scope.back().emplace(key, std::move(value));
    }
    else {
      base::scope.back().insert_or_assign(key, std::move(value));
    }
  }
  
  void Set(K&& key, V&& value) {
    if constexpr(!allow_multiple_assignment) {
      if (HasTop(key)) {
        throw cotyl::FormatExceptStr("Redefinition of %s", key);
      }
      base::scope.back().emplace(std::move(key), std::move(value));
    }
    else {
      base::scope.back().insert_or_assign(std::move(key), std::move(value));
    }
  }
  
  void Set(K&& key, const V& value) {
    if constexpr(!allow_multiple_assignment) {
      if (HasTop(key)) {
        throw cotyl::FormatExceptStr("Redefinition of %s", key);
      }
      base::scope.back().emplace(std::move(key), value);
    }
    else {
      base::scope.back().insert_or_assign(std::move(key), value);
    }
  }

  bool Has(const K& key) const {
    for (auto s = base::scope.rbegin(); s != base::scope.rend(); s++) {
      if (s->contains(key)) {
        return true;
      }
    }
    return false;
  }

  bool HasTop(const K& key) const {
    return base::scope.back().contains(key);
  }

  V& Get(const K& key) {
    for (auto s = base::scope.rbegin(); s != base::scope.rend(); s++) {
      if (s->contains(key)) {
        return s->at(key);
      }
    }
    throw cotyl::FormatExceptStr("Invalid scope access: %s", key);
  }

  const V& Get(const K& key) const {
    for (auto s = base::scope.rbegin(); s != base::scope.rend(); s++) {
      if (s->contains(key)) {
        return s->at(key);
      }
    }
    throw cotyl::FormatExceptStr("Invalid scope access: %s", key);
  }
};


template<typename K>
struct SetScope : public Scope<cotyl::unordered_set<K>> {
  using base = Scope<cotyl::unordered_set<K>>;

  void Add(const K& key) {
    if (base::scope.back().contains(key)) {
      throw cotyl::FormatExceptStr("Redefinition of %s", key);
    }
    base::scope.back().insert(key);
  }

  bool Has(const K& key) const {
    for (auto s = base::scope.rbegin(); s != base::scope.rend(); s++) {
      if (s->contains(key)) {
        return true;
      }
    }
    return false;
  }

  bool HasTop(const K& key) const {
    return base::scope.back().contains(key);
  }
};



template<typename K>
struct VecScope : public Scope<cotyl::vector<K>> {
  using base = Scope<cotyl::vector<K>>;

  void Push(const K& key) {
    base::scope.back().push_back(key);
  }

  void End() const {
    return base::scope.back().back();
  }
};

}