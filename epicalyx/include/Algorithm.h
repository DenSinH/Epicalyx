#pragma once


namespace epi::cotyl {

// https://en.cppreference.com/w/cpp/container/unordered_set/erase_if
template<typename T, class Pred>
size_t erase_if(T& container, const Pred& predicate) {
  const size_t old_size = container.size();
  for (auto it = container.begin(); it != container.end();) {
    if (predicate(*it)) {
      it = container.erase(it);
    }
    else {
      it++;
    }
  }
  return old_size - container.size();
}

}