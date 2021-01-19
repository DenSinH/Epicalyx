#ifndef EPICALYX_UTILS_H
#define EPICALYX_UTILS_H

template <typename T>
struct Is {
    constexpr Is(T value) : value(value) {}

    template <T... values>
    [[nodiscard]] constexpr bool AnyOf() const {
//        const std::set<T> _values = {values...};
//        return _values.contains(value);
        return ((value == values) || ...);
    }

private:
    T value;
};

#endif //EPICALYX_UTILS_H
