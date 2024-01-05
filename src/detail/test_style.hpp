#pragma once

#include "src/detail/priority.hpp"
#include "src/detail/static_closure.hpp"

#include <optional>
#include <type_traits>

namespace skytest::detail {

struct test_style
{
  struct runtime_only
  {
    template <class>
    static constexpr auto value = std::optional<bool>{};
  };

  struct compile_time_if_possible
  {
  private:
    template <
        class F,
        std::enable_if_t<is_static_closure_v<F>, bool> result = bool{F{}()}>
    static constexpr auto try_eval(priority<1>)
    {
      return std::optional<bool>{result};
    }
    template <class F>
    static constexpr auto try_eval(priority<0>)
    {
      return std::optional<bool>{};
    }

  public:
    template <class F>
    static constexpr auto value = try_eval<F>(priority<1>{});
  };

  struct compile_time
  {
    template <class F>
    static constexpr auto value = std::optional<bool>{bool{F{}()}};
  };
};

}  // namespace skytest::detail
