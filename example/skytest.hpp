#pragma once

#include "_virtual_includes/skytest/skytest/skytest.hpp"

#include <iostream>

template <>
// NOLINTNEXTLINE(cppcoreguidelines-interfaces-global-init,misc-definitions-in-headers)
const auto ::skytest::cfg<::skytest::override> =
    ::skytest::runner<>{std::cout, ::skytest::colors{{}, {}, {}, {}}};
