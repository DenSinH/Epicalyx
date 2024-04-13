#include <type_traits>


namespace epi::cotyl {

template<template<typename...> class TT, typename T>
struct is_instantiation_of : std::false_type { };

template<template<typename...> class TT, typename... Ts>
struct is_instantiation_of<TT, TT<Ts...>> : std::true_type { };

template<template<typename...> class TT, typename T>
constexpr bool is_instantiation_of_v = is_instantiation_of<TT, T>::value; 

}