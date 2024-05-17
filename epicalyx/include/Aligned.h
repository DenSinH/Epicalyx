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
using unique_ptr_aligned = std::unique_ptr<T, detail::AlignedDeleter<T>>;

template<typename T>
inline unique_ptr_aligned<T> aligned_uptr(std::size_t align, std::size_t size) {
  return unique_ptr_aligned<T>(
    static_cast<T*>(std::aligned_alloc(align, sizeof(T) * size)), {}
  );
}

}
