#pragma once

#include "calyx/Types.h"
#include "types/TypeFwd.h"

namespace epi::detail {

calyx::Global GetGlobalValue(const type::AnyType& type);
calyx::Local MakeLocal(loc_index_t loc_idx, const type::AnyType& type);

}