#pragma once

namespace skytest {
namespace detail {

template <int N>
struct priority : priority<N - 1>
{};
template <>
struct priority<0>
{};

}  // namespace detail
}  // namespace skytest
