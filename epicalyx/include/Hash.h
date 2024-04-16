#pragma once

#ifdef USE_BOOST

#include <functional>
#include <type_traits>
#include <utility>
#include <boost/container_hash/hash.hpp>

#include <type_traits>
#include <utility>

namespace epi {

template <typename T>
concept std_hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept boost_hashable = requires(T a) {
    { boost::hash_value(a) } -> std::convertible_to<std::size_t>;
};

template <typename T>
constexpr bool std_hashable_v = std_hashable<T>;

template <typename T>
constexpr bool boost_hashable_v = boost_hashable<T>;

template <typename T>
requires (std_hashable<T> && !boost_hashable<T>)
std::size_t hash_value(const T& arg) {
    return std::hash<T>{}(arg);
}

namespace calyx {

template <class T>
inline void hash_combine(std::size_t& seed, T const& v) {
  boost::hash_combine<T>(seed, v);
}

}

}

#else

namespace epi::calyx {

// template <class T>
// inline void hash_combine(std::size_t& seed, T const& v) {
#error "Unimplemented definition of hash_combine"
// }

}

#endif