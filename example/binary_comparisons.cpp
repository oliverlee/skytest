#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::ge;
  using ::skytest::gt;
  using ::skytest::le;
  using ::skytest::lt;
  using ::skytest::ne;

  "eq"_test = [] { return expect(eq(1, 1)); };
  "ne"_test = [] { return expect(ne(1, 0)); };
  "lt"_test = [] { return expect(lt(0, 1)); };
  "gt"_test = [] { return expect(gt(1, 0)); };
  "le"_test = [] { return expect(le(1, 1)); };
  "ge"_test = [] { return expect(ge(1, 1)); };
}
