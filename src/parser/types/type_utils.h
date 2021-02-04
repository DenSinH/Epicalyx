#ifndef EPICALYX_TYPE_UTILS_H
#define EPICALYX_TYPE_UTILS_H

template<typename T>
struct type_string {
    const static inline std::string value;
};

template<> struct type_string<char> { const static inline std::string value = "char"; };
template<> struct type_string<unsigned char> { const static inline std::string value = "unsigned char"; };
template<> struct type_string<short> { const static inline std::string value = "short"; };
template<> struct type_string<unsigned short> { const static inline std::string value = "unsigned short"; };
template<> struct type_string<int> { const static inline std::string value = "int"; };
template<> struct type_string<unsigned int> { const static inline std::string value = "unsigned int"; };
template<> struct type_string<long> { const static inline std::string value = "long"; };
template<> struct type_string<unsigned long> { const static inline std::string value = "long"; };
template<> struct type_string<long long> { const static inline std::string value = "long long"; };
template<> struct type_string<unsigned long long> { const static inline std::string value = "unsigned long long"; };
template<> struct type_string<float> { const static inline std::string value = "float"; };
template<> struct type_string<double> { const static inline std::string value = "double"; };

template<typename T> inline const std::string type_string_v = type_string<T>::value;

#endif //EPICALYX_TYPE_UTILS_H
