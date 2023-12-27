# skytest

A non-allocating[^1] C++17+ unit test framework.

## overview

A minimal unit test example

```cpp:example/minimal_pass.cpp
#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::expect;

  "truthy"_test = [] { return expect(true); };
}

```

when run, will print

```console:example/minimal_pass.log
test `truthy`...[CONSTEXPR PASS]
all tests passed (1 test)

```

A test that fails

```cpp:example/minimal_fail.cpp
#include "skytest/skytest.hpp"

auto main(int argc, char*[]) -> int
{
  using namespace skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;

  "falsy"_test = [&] { return expect(eq(0, argc)); };
}

```

will print

```console:example/minimal_fail.log
test `falsy`...[FAIL] example/minimal_fail.cpp:9
(0 == 1)

0 tests passed | 1 test failed

```

When running tests, `skytest` will attempt to invoke test closures at
compile-time. If able to, results will be classified `CONSTEXPR PASS` or
`CONSTEXPR FAIL` instead of `PASS` or `FAIL`.

## examples

#### binary comparisons
<details><summary></summary>


Binary comparison predicates display arguments on failure.

```cpp:example/binary_comparisons.cpp
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

```

</details>


#### logical operators
<details><summary></summary>


Logical operators can be used to compose predicates - including user defined predicates.

```cpp:example/logical_operations.cpp
#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::expect;
  using ::skytest::lt;

  "and"_test = [] { return expect(lt(1, 2) and lt(2, 3)); };
  "or"_test = [] { return expect(lt(3, 2) or lt(2, 3)); };
  "not"_test = [] { return expect(not lt(1, 0)); };
}

```

</details>


#### additional output on failure
<details><summary></summary>


Additional output can be displayed on test failure.

```cpp:example/additional_output.cpp
#include "skytest/skytest.hpp"

#include <utility>

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;

  "string-view-ish"_test = [] { return expect(eq(1, 2), "a message"); };

  "ostream invocable closure"_test = [] {
    const auto x = std::pair{1, 2};
    const auto y = std::pair{2, 1};
    return expect(eq(x, y), [=](auto& os) {
      os << x.first << ", " << y.first << "\n";
    });
  };
}

```
```console:example/additional_output.log
test `string-view-ish`...[CONSTEXPR FAIL] example/additional_output.cpp:11
(1 == 2)
a message
test `ostream invocable closure`...[CONSTEXPR FAIL] example/additional_output.cpp:16
(std::pair<int, int>{...} == std::pair<int, int>{...})
1, 2

0 tests passed | 2 tests failed

```

NOTE: The message closure is not invoked within the test closure. Capturing
`x` and `y` by reference will result in dangling.
</details>


#### user defined predicates
<details><summary></summary>


Defining a predicate with `pred` captures and displays arguments on failure.

```cpp:example/user_defined_predicates.cpp
#include "skytest/skytest.hpp"

#include <array>
#include <iterator>

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::expect;
  using ::skytest::pred;

  static constexpr auto empty = pred([](const auto& rng) {
    return std::empty(rng);
  });

  "empty array"_test = [] { return expect(empty(std::array{1, 2, 3})); };
}

```
```console:example/user_defined_predicates.log
test `empty array`...[CONSTEXPR FAIL] example/user_defined_predicates.cpp:16
(lambda at example/user_defined_predicates.cpp:12:38){}([1, 2, 3])

0 tests passed | 1 test failed

```

The description of user defined predicates can be customized

```cpp:example/described_predicates.cpp

#include "skytest/skytest.hpp"

#include <array>
#include <string_view>

using namespace skytest::literals;
using ::skytest::expect;
using ::skytest::pred;

struct empty_desc
{
  using notation_type = skytest::notation::function;
  static constexpr auto symbol = std::string_view{"empty"};
};

static constexpr auto empty = pred(empty_desc{}, [](const auto& rng) {
  return std::empty(rng);
});

auto main() -> int
{

  "empty array"_test = [] { return expect(empty(std::array{1, 2, 3})); };
}

```
```console:example/described_predicates.log
test `empty array`...[CONSTEXPR FAIL] example/described_predicates.cpp:24
empty([1, 2, 3])

0 tests passed | 1 test failed

```

C++20 enables a terser syntax.

```cpp:example/described_predicates_20.cpp
#include "skytest/skytest.hpp"

#include <array>

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::expect;
  using ::skytest::function;
  using ::skytest::pred;

  static constexpr auto empty = pred(function<"∅">, std::ranges::empty);

  "empty array"_test = [] { return expect(empty(std::array{1, 2, 3})); };
}

```

</details>


#### parameterized tests
<details><summary></summary>


Tests can be parameterized by type

```cpp:example/type_parameterized.cpp
#include "skytest/skytest.hpp"

#include <cmath>
#include <complex>
#include <tuple>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::types;

  "typed"_test * std::tuple<int, float, std::complex<double>>{} =  //
      [](auto param) { return expect(eq(0, std::abs(param))); };

  "with types param"_test * types<int, float> =  //
      [](auto type_id) {
        using T = typename decltype(type_id)::type;
        return expect(eq(T{}, T{} + T{}));
      };
}

```

or by value

```cpp:example/value_parameterized.cpp
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

```

If parameters are defined as a static constant, `param_ref` may enable
compile-time tests.

```cpp:example/param_ref_parameterized.cpp
#include "skytest/skytest.hpp"

#include <array>
#include <tuple>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::lt;
  using ::skytest::param_ref;

  static constexpr auto type_params = std::tuple{1.0F, 1.0};

  "types with param_ref"_test * param_ref<type_params> =  //
      [](auto param) { return expect(eq(1, param)); };

  static constexpr auto value_params = std::array{1, 2, 3};

  "values with param_ref"_test * param_ref<value_params> =  //
      [](auto param) { return expect(lt(0, param)); };
}

```

If parameters are defined as a literal-type[^2], C++20 allows use of `param`:

```cpp:example/param_parameterized.cpp
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

```

</details>


#### aborts
<details><summary></summary>


Check that abort is called (e.g. in a function precondition).

```cpp:example/aborts.cpp
#include "skytest/skytest.hpp"

#include <cassert>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::aborts;
  using ::skytest::expect;

  static const auto not_zero = [](int value) { assert(value != 0); };

  "aborts"_test = [] { return expect(aborts([] { not_zero(0); })); };
}

```

</details>


[^1]: The default printer uses `std::cout` and `skytest::aborts` calls `fork`.
[^2]: https://en.cppreference.com/w/cpp/named_req/LiteralType
