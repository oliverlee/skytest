#pragma once

#include "_virtual_includes/skytest/skytest/skytest.hpp"

#include <iostream>

template <>
const auto ::skytest::cfg<::skytest::override> =
    ::skytest::runner<>{std::cout, ::skytest::colors{{}, {}, {}, {}}};
