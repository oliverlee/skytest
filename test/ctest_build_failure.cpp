#include "skytest/skytest.hpp"

auto x = false;

auto main() -> int
{
    using namespace ::skytest::literals;
    using ::skytest::expect;
    using ::skytest::types;

    "test1"_ctest = [] {
        return expect(x);
    };

    "test2"_ctest * types<int> = [](auto) {
        return expect(x);
    };
}
