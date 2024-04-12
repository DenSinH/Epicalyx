#pragma once

#include <variant>
#include <array>
#include <type_traits>
#include <tuple>


namespace epi::cotyl {

template <typename... Ts>
struct pack { 
  static constexpr std::size_t size = sizeof...(Ts);
};

namespace detail {

template <typename T0, typename...>
struct flatten_helper { using type = T0; };

template <typename ... Ts1, typename T0, typename ... Ts2>
struct flatten_helper<pack<Ts1...>, T0, Ts2...>
   : flatten_helper<pack<Ts1..., T0>, Ts2...>
 { };

template <typename ... Ts1, typename ... Ts2, typename ... Ts3>
struct flatten_helper<pack<Ts1...>, pack<Ts2...>, Ts3...>
   : flatten_helper<pack<Ts1...>, Ts2..., Ts3...>
 { };

}

template <typename... Ts>
using flatten_t = typename detail::flatten_helper<pack<>, Ts...>::type;

template<template<typename T> class Op, typename Types>
struct map_types;

template<template<typename T> class Op, typename... Args>
struct map_types<Op, pack<Args...>> {
    using mapped = pack<Op<Args>...>;
};

template<template<typename T> class Op, typename Types>
using map_types_t = typename map_types<Op, Types>::mapped;

template<template<typename... Args> class Op, typename Types>
struct map_pack;

template<template<typename... Args> class Op, typename... Args>
struct map_pack<Op, pack<Args...>> {
    using mapped = Op<Args...>;
};

template<template<typename... Args> class Op, typename Types>
using map_pack_t = typename map_pack<Op, Types>::mapped;

}