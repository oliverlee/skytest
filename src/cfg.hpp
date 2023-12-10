#pragma once

#include "src/default_printer.hpp"
#include "src/runner.hpp"

#include <iostream>

namespace skytest {

struct override
{};

template <class = override>
auto cfg = runner<default_printer>{std::cout};

}  // namespace skytest
