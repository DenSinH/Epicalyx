#pragma once

#include "Default.h"

namespace epi {

template<typename T>
struct type_string {
  const static inline std::string value;
};

template<> struct type_string<i8> { const static inline std::string value = "char"; };
template<> struct type_string<u8> { const static inline std::string value = "unsigned char"; };
template<> struct type_string<i16> { const static inline std::string value = "short"; };
template<> struct type_string<u16> { const static inline std::string value = "unsigned short"; };
template<> struct type_string<i32> { const static inline std::string value = "int"; };
template<> struct type_string<u32> { const static inline std::string value = "unsigned int"; };
template<> struct type_string<i64> { const static inline std::string value = "long long"; };
template<> struct type_string<u64> { const static inline std::string value = "unsigned long long"; };
template<> struct type_string<float> { const static inline std::string value = "float"; };
template<> struct type_string<double> { const static inline std::string value = "double"; };

template<typename T> inline const std::string type_string_v = type_string<T>::value;

}
