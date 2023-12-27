#pragma once

#include "src/runner.hpp"

#include <iostream>

namespace skytest {

struct override
{};

template <class = override>
// NOLINTNEXTLINE(cppcoreguidelines-interfaces-global-init)
const auto cfg = runner<>{std::cout};

}  // namespace skytest
