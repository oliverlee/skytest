#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::ge;
  using ::skytest::gt;
  using ::skytest::le;
  using ::skytest::lt;
  using ::skytest::ne;

  "expect true"_test = [] { return expect(true); };
  "expect eq"_test = [] { return expect(eq(1, 1)); };
  "expect ne"_test = [] { return expect(ne(1, 0)); };
  "expect lt"_test = [] { return expect(lt(0, 1)); };
  "expect gt"_test = [] { return expect(gt(1, 0)); };
  "expect le"_test = [] { return expect(le(0, 1)); };
  "expect ge"_test = [] { return expect(ge(1, 0)); };

  "not expect true"_test = [] { return expect(false); };
  "not expect eq"_test = [] { return expect(eq(1, 0)); };
  "not expect ne"_test = [] { return expect(ne(1, 1)); };
  "not expect lt"_test = [] { return expect(lt(1, 0)); };
  "not expect gt"_test = [] { return expect(gt(0, 1)); };
  "not expect le"_test = [] { return expect(le(1, 0)); };
  "not expect ge"_test = [] { return expect(ge(0, 1)); };
}
