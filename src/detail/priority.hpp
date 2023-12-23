#pragma once

namespace skytest::detail {

template <int N>
struct priority : priority<N - 1>
{};
template <>
struct priority<0>
{};

}  // namespace skytest::detail
