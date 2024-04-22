#pragma once

#include <type_traits>

namespace epi::cotyl {

template<typename T>
using base_t = std::remove_const_t<std::remove_reference_t<T>>;

}

#define decltype_t(val) ::epi::cotyl::base_t<decltype(val)>
