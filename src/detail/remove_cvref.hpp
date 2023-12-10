#pragma once

#include <type_traits>

namespace skytest {
namespace detail {

template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

}
}  // namespace skytest
