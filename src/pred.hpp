#pragma once

#include "src/detail/predicate.hpp"
#include "src/utility.hpp"
#include "src/version.hpp"

#include <functional>
#include <string_view>

#if SKYTEST_CXX20
#include <array>
#include <cstddef>
#endif

namespace skytest {
namespace detail {

template <class Format = predicate_format<>>
struct pred_fn
{
  template <class F>
  constexpr auto operator()(F&& f) const
  {
    return predicate<remove_cvref_t<F>, type_name<F>, Format>{
        std::forward<F>(f)};
  }
  template <class T, class F>
  constexpr auto operator()(T, F&& f) const
  {
    return pred_fn<T>{}(std::forward<F>(f));
  }
};

struct pred_fmt
{
  static constexpr struct eq_
  {
    using notation_type = notation::infix;
    static constexpr auto symbol = std::string_view{"=="};
  } eq{};
  static constexpr struct ne_
  {
    using notation_type = notation::infix;
    static constexpr auto symbol = std::string_view{"!="};
  } ne{};
  static constexpr struct lt_
  {
    using notation_type = notation::infix;
    static constexpr auto symbol = std::string_view{"<"};
  } lt{};
  static constexpr struct gt_
  {
    using notation_type = notation::infix;
    static constexpr auto symbol = std::string_view{">"};
  } gt{};
  static constexpr struct le_
  {
    using notation_type = notation::infix;
    static constexpr auto symbol = std::string_view{"<="};
  } le{};
  static constexpr struct ge_
  {
    using notation_type = notation::infix;
    static constexpr auto symbol = std::string_view{">="};
  } ge{};
};

}  // namespace detail

inline constexpr auto pred = detail::pred_fn<>{};

inline constexpr auto eq = pred(detail::pred_fmt::eq, std::equal_to<>{});
inline constexpr auto ne = pred(detail::pred_fmt::ne, std::not_equal_to<>{});
inline constexpr auto lt = pred(detail::pred_fmt::lt, std::less<>{});
inline constexpr auto gt = pred(detail::pred_fmt::gt, std::greater<>{});
inline constexpr auto le = pred(detail::pred_fmt::le, std::less_equal<>{});
inline constexpr auto ge = pred(detail::pred_fmt::ge, std::greater_equal<>{});

#if SKYTEST_CXX20
template <std::size_t N>
struct string_literal
{
  std::array<char, N> chars;

  constexpr string_literal(const char (&s)[N])
  {
    for (auto i = std::size_t{}; i != N; ++i) {
      chars[i] = s[i];
    }
  }

  constexpr operator std::string_view() const
  {
    return {chars.begin(), chars.size()};
  }
};
template <string_literal symbol>
struct string_literal_constant
{
  static constexpr auto value = std::string_view{symbol};
};
template <string_literal symbol>
constexpr auto infix =
    predicate_format<string_literal_constant<symbol>, notation::infix>{};
template <string_literal symbol>
constexpr auto function =
    predicate_format<string_literal_constant<symbol>, notation::function>{};
#endif

}  // namespace skytest
