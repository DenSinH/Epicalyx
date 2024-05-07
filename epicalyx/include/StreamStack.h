#include "Stream.h"
#include "TypeTraits.h"
#include "Vector.h"

namespace epi::cotyl {

template<typename T, typename S>
requires (std::is_base_of_v<Stream<S>, T>)
struct StreamStack : Stream<S> {
  using stack_t = cotyl::vector<T>;
  using const_iterator = stack_t::const_iterator;

  StreamStack() = delete;
  StreamStack(T&& base, S&& separator) : 
      stack{}, separator{std::move(separator)} { 
    stack.emplace_back(std::move(base));
  }

  void Push(T&& value) { stack.emplace_back(std::move(value)); }
  const T& Top() const { return stack.back(); }
  T& Top() { return stack.back(); }
  const std::size_t Size() const { return stack.size(); }
  
  const_iterator begin() const { return stack.begin(); }
  const_iterator end() const { return stack.end(); }

  void PrintLoc(std::ostream& out) const {
    for (const auto& layer : stack) {
      layer.PrintLoc(out);
      out << std::endl;
    }
  }

protected:
  S GetNew() final {
    if (stack.empty()) throw EOSError();
    if (stack.back().EOS()) {
      stack.pop_back();
      return separator;
    }
    return stack.back().Get();
  }

  bool IsEOS() final {
    return stack.empty();
  }

private:
  S separator;
  stack_t stack;
};

}