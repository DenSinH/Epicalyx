#pragma once

#include <cstdlib>
#include <memory>

namespace epi::cotyl {

namespace detail {

template<typename T>
struct AlignedDeleter {
  void operator()(T* data) const {
    std::free(data);
  }
};

}

template<typename T>
using aligned_uptr = std::unique_ptr<T, detail::AlignedDeleter<T>>;

template<typename T>
inline aligned_uptr<T> make_ualigned(std::size_t align, std::size_t size) {
  return aligned_uptr<T>(
    static_cast<T*>(std::calloc(size, sizeof(T))), {}
  );
}

}
