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

[^1]: The default printer uses `std::cout` and `skytest::aborts` calls `fork`.
