#include "skytest/skytest.hpp"

auto main() -> int
{
    using namespace ::skytest::literals;
    using ::skytest::eq;
    using ::skytest::expect;

    "noreturn"_test = [] {
        // not returned in test closure
        expect(eq(1, 1));

        return expect(eq(1, 1));
    };
}
