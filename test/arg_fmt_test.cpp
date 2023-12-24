#include "skytest/skytest.hpp"

#include <array>
#include <cstddef>
#include <iterator>
#include <ostream>
#include <string_view>

struct empty_desc
{
  using notation_type = skytest::notation::function;
  static constexpr auto symbol = std::string_view{"empty"};
};
constexpr auto empty = skytest::pred(empty_desc{}, [](const auto& rng) {
  return std::empty(rng);
});

template <class T, std::size_t N>
struct array : std::array<T, N>
{
  friend auto& operator<<(std::ostream& os, const array& arr)
  {
    for (const auto& x : arr) {
      os << "{" << x << "}";
    }
    return os;
  }
};

struct wrapped
{
  int value;

  friend constexpr auto operator==(wrapped, wrapped) { return false; }
};

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;

  "array uses default range arg fmt"_test = [] {
    return expect(empty(std::array{1, 2, 3}));
  };

  "custom array uses custom fmt"_test = [] {
    return expect(empty(array<int, 2>{4, 5}));
  };

  "type without operator<< is displayed"_test = [] {
    return expect(eq(wrapped{1}, wrapped{2}));
  };
}
