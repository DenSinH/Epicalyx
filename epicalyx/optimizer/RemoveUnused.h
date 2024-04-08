#pragma once


namespace epi {

namespace calyx {
struct Function;
}

size_t RemoveUnused(calyx::Function& program);

}