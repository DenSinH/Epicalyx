#pragma once

#include <memory>

namespace epi::cotyl {

template<typename To, typename From>
constexpr const To* unique_ptr_cast(const std::unique_ptr<From>& ptr) {
  return static_cast<To*>(ptr.get());
}

template<typename To, typename From>
constexpr To* unique_ptr_cast(std::unique_ptr<From>& ptr) {
  return static_cast<To*>(ptr.get());
}

template<typename To, typename From>
constexpr const To* shared_ptr_cast(const std::shared_ptr<From>& ptr) {
  return static_cast<To*>(ptr.get());
}

template<typename To, typename From>
constexpr To* shared_ptr_cast(std::shared_ptr<From>& ptr) {
  return static_cast<To*>(ptr.get());
}

}