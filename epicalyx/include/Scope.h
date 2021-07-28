#pragma once

#include <map>
#include <vector>
#include <unordered_set>
#include <stdexcept>

namespace epi::cotyl {

struct Scope {

  virtual void NewLayer() = 0;
  virtual void PopLayer() = 0;
  virtual size_t Depth() const = 0;

  template<typename T>
  auto operator<<(const T& callable) {
    NewLayer();
    auto result = callable();
    PopLayer();
    return std::move(result);
  }

};

template<typename K, typename V>
struct MapScope : public Scope {

  const std::map<K, V>& Base() {
    return scope[0];
  }

  void NewLayer() final {
    scope.emplace_back();
  }

  void PopLayer() final {
    scope.pop_back();
  }

  size_t Depth() const final { return scope.size(); }

  void Set(const K& key, const V& value) {
    if (scope.back().contains(key)) {
      throw cotyl::FormatExceptStr("Redefinition of %s", key);
    }
    scope.back()[key] = value;
  }

  bool Has(const K& key) const {
    for (auto s = scope.rbegin(); s != scope.rend(); s++) {
      if (s->contains(key)) {
        return true;
      }
    }
    return false;
  }

  bool HasTop(const K& key) const {
    return scope.back().contains(key);
  }

  V Get(const K& key) const {
    for (auto s = scope.rbegin(); s != scope.rend(); s++) {
      if (s->contains(key)) {
        return s->at(key);
      }
    }
    throw cotyl::FormatExceptStr("Invalid scope access: %s", key);
  }

private:
  std::vector<std::map<K, V>> scope{{}};
};


template<typename K>
struct SetScope : public Scope {

  const std::unordered_set<K>& Base() {
    return scope[0];
  }

  void NewLayer() final {
    scope.emplace_back();
  }

  void PopLayer() final {
    scope.pop_back();
  }

  size_t Depth() const final { return scope.size(); }

  void Add(const K& key) {
    if (scope.back().contains(key)) {
      throw cotyl::FormatExceptStr("Redefinition of %s", key);
    }
    scope.back().insert(key);
  }

  bool Has(const K& key) const {
    for (auto s = scope.rbegin(); s != scope.rend(); s++) {
      if (s->contains(key)) {
        return true;
      }
    }
    return false;
  }

  bool HasTop(const K& key) const {
    return scope.back().contains(key);
  }

private:
  std::vector<std::unordered_set<K>> scope{{}};
};



template<typename K>
struct VecScope : public Scope {

  const std::unordered_set<K>& Base() {
    return scope[0];
  }

  void NewLayer() final {
    scope.emplace_back();
  }

  void PopLayer() final {
    scope.pop_back();
  }

  size_t Depth() const final { return scope.size(); }

  void Push(const K& key) {
    scope.back().push_back(key);
  }

  void End() const {
    return scope.back().back();
  }

private:
  std::vector<std::vector<K>> scope{{}};
};

}