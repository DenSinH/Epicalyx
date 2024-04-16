#pragma once

#include <cstddef>


namespace epi {

namespace calyx {
struct Function;
}

std::size_t RemoveUnused(calyx::Function& program);

}