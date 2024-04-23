#pragma once

#include "swl/variant.hpp"
#include "Format.h"
#include "Default.h"

#ifdef USE_BOOST

#include <boost/container/set.hpp>
#include <boost/container/map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/container/vector.hpp>

#include <boost/container_hash/hash.hpp>

#else

#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#endif

#include <functional>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include "Vector.h"