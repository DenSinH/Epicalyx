#pragma once

#include <boost/container/vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/small_vector.hpp>

namespace epi::cotyl {

template<typename T>
using vector = boost::container::vector<T>;
template<typename T, std::size_t N>
using static_vector = boost::container::static_vector<T, N>;
template<typename T, std::size_t N>
using small_vector = boost::container::small_vector<T, N>;


}