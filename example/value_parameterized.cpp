#include "skytest/skytest.hpp"

#include <vector>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;
  using ::skytest::lt;

  "values"_test * std::vector{1, 2, 3} =  //
      [](auto param) { return expect(lt(0, param)); };
}
