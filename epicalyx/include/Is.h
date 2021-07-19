#pragma once


namespace epi::calyx {

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

}