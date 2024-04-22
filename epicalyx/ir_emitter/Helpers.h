#pragma once

#include "calyx/Types.h"
#include "types/TypeFwd.h"

namespace epi::detail {

calyx::global_t GetGlobalValue(const type::AnyType& type);
std::pair<calyx::Local::Type, u64> GetLocalType(const type::AnyType& type);

}