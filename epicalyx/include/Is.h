#pragma once


namespace epi::cotyl {

template <typename T>
struct Is {
  constexpr Is(T value) : value(value) {}

  template <T... values>
  [[nodiscard]] constexpr bool AnyOf() const {
    return ((value == values) || ...);
  }

private:
  T value;
};

template <typename T, typename ...Ts>
inline constexpr bool are_all_same_v = std::conjunction_v<std::is_same<T,Ts>...>;

}