#pragma once

#include <type_traits>
#include <cstddef>


namespace epi::cotyl {

template<template<typename...> class TT, typename T>
struct is_instantiation_of : std::false_type { };

template<template<typename...> class TT, typename... Ts>
struct is_instantiation_of<TT, TT<Ts...>> : std::true_type { };

template<template<typename...> class TT, typename T>
static constexpr bool is_instantiation_of_v = is_instantiation_of<TT, T>::value; 

template <typename T, typename... Ts>
struct type_index;

template <typename T, typename... Ts>
struct type_index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct type_index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + type_index<T, Ts...>::value> {};

template <typename T, typename... Ts>
static constexpr std::size_t type_index_v = type_index<T, Ts...>::value;

}