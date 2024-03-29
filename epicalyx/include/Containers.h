#pragma once

#ifdef USE_BOOST

#include <boost/container/set.hpp>
#include <boost/container/map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

namespace epi::cotyl {

template<typename T>
using set = boost::container::set<T>;
template<typename K, typename V>
using map = boost::container::map<K, V>;

template<typename T>
struct unordered_set final : public boost::unordered_set<T> {
  using boost::unordered_set<T>::unordered_set;

  bool contains(const T& key) const {
    return this->find(key) != this->end();
  }
};

template<typename K, typename V>
struct unordered_map final : public boost::unordered_map<K, V> {
  using boost::unordered_map<K, V>::unordered_map;

  bool contains(const K& key) const {
    return this->find(key) != this->end();
  }
};

template<typename T>
using flat_set = boost::container::flat_set<T>;
template<typename K, typename V>
using flat_map = boost::container::flat_map<K, V>;
template<typename T, std::size_t N>
using static_vector = boost::container::static_vector<T, N>;

}

#else

#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace epi::cotyl {

template<typename T>
using set = std::set<T>;
template<typename K, typename V>
using map = std::map<K, V>;
template<typename T>
using unordered_set = std::unordered_set<T>;
template<typename K, typename V>
using unordered_map = std::unordered_map<K, V>;
template<typename T>
using flat_set = std::unordered_set<T>;
template<typename K, typename V>
using flat_map = std::unordered_map<K, V>;

}

#endif

namespace epi::cotyl {

template<typename K, typename T>
T& get_default(unordered_map<K, T>& graph, const K& key) {
  if (!graph.contains(key)) {
    graph[key] = {};
  }
  return graph[key];
}

}


namespace std {

// basic hash for std::pair s
template<typename T1, typename T2>
struct hash<std::pair<T1, T2>> {
  size_t operator()(const std::pair<T1, T2>& pair) const {
    return hash<T1>()(pair.first) ^ hash<T2>()(pair.second);
  }
};

}
