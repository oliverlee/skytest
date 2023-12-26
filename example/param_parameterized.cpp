#include "skytest/skytest.hpp"

#include <array>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;
  using ::skytest::lt;
  using ::skytest::param;

  "with paramf"_test * param<std::array{1, 2, 3}> =  //
      [](auto param) { return expect(lt(0, param)); };
}
