#pragma once

#include <array>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

namespace skytest {
namespace detail {
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <class F, class... Args>
struct relation
{
  using predicate_type = F;
  using arguments_type = std::tuple<Args...>;

  arguments_type args;
  bool value;

  constexpr operator bool() const { return value; }
};

template <class F>
struct predicate : F
{
  template <class G, class = std::enable_if_t<std::is_constructible_v<F, G&&>>>
  constexpr predicate(G&& g) : F{std::forward<G>(g)}
  {}

  template <class... Ts>
  constexpr auto operator()(Ts&&... args) const -> std::remove_reference_t<
      decltype(bool(std::declval<const F&>()(std::as_const(args)...)),
               std::declval<relation<F, remove_cvref_t<Ts>...>>())>
  {
    const auto value = static_cast<const F&>(*this)(std::as_const(args)...);
    return {std::tuple<remove_cvref_t<Ts>...>{args...}, value};
  }
};

template <class F>
static constexpr auto predicate_type_name()
{
  constexpr auto full = std::string_view{__PRETTY_FUNCTION__};

#ifdef __clang__
  constexpr auto prefix = std::string_view{"F = "};
  constexpr auto suffix = std::string_view{"]"};
#else
  constexpr auto prefix = std::string_view{"with F = "};
  constexpr auto suffix = std::string_view{"]"};
#endif

  const auto lower = full.find(prefix) + prefix.size();
  const auto upper = full.find(suffix, lower);

  return std::string_view{full.begin() + lower, upper - lower};
}

class test;

template <class T, class = void>
struct has_args : std::false_type
{};
template <class T>
struct has_args<T, std::void_t<decltype(std::declval<const T&>().args)>>
    : std::true_type
{};
template <class T>
constexpr auto has_args_v = has_args<T>::value;

inline constexpr auto empty_args = std::tuple<>{};

template <class Relation, std::enable_if_t<has_args_v<Relation>, bool> = true>
constexpr auto pred_name(const Relation&)
{
  return predicate_type_name<typename Relation::predicate_type>();
}
template <
    class Relation,
    std::enable_if_t<not has_args_v<Relation>, bool> = true>
constexpr auto pred_name(const Relation&)
{
  return std::string_view{};
}

template <class Relation, std::enable_if_t<has_args_v<Relation>, bool> = true>
constexpr auto& args(const Relation& r)
{
  return r.args;
}
template <
    class Relation,
    std::enable_if_t<not has_args_v<Relation>, bool> = true>
constexpr auto& args(const Relation&)
{
  return empty_args;
}
}  // namespace detail

class source_location
{
  std::string_view file_{"unknown"};
  int line_{-1};

public:
  static constexpr auto current(
      const char* file = __builtin_FILE(), int line = __builtin_LINE()) noexcept
  {
    auto sl = source_location{};
    sl.file_ = file;
    sl.line_ = line;
    return sl;
  }

  constexpr auto file_name() const noexcept { return file_; }
  constexpr auto line() const noexcept { return line_; }
};

template <class Relation>
struct result
{
  Relation relation;
  source_location source;
  std::string_view name{"unknown"};

  constexpr explicit operator bool() const { return bool(relation); }

  constexpr auto pred_name() const { return detail::pred_name(relation); }
  constexpr auto& arguments() const { return detail::args(relation); }
};

template <class Relation>
constexpr auto
expect(Relation r, source_location sl = source_location::current())
{
  return result<Relation>{r, sl};
}

template <class F>
constexpr auto pred(F&& f)
{
  return detail::predicate<detail::remove_cvref_t<F>>{std::forward<F>(f)};
}

inline constexpr auto eq = pred(std::equal_to<>{});
inline constexpr auto ne = pred(std::not_equal_to<>{});
inline constexpr auto lt = pred(std::less<>{});
inline constexpr auto gt = pred(std::greater<>{});
inline constexpr auto le = pred(std::less_equal<>{});
inline constexpr auto ge = pred(std::greater_equal<>{});

struct summary
{
  std::size_t pass{};
  std::size_t fail{};
};

class default_printer
{
  std::ostream& os_;

  struct colors
  {
    static constexpr auto none = std::string_view{"\033[0m"};
    static constexpr auto pass = std::string_view{"\033[32m"};
    static constexpr auto fail = std::string_view{"\033[31m"};
  };

  struct tests
  {
    std::size_t count;

    friend auto& operator<<(std::ostream& os, const tests& t)
    {
      os << t.count;
      os << ((t.count == 1U) ? " test" : " tests");
      return os;
    }
  };

  template <class... Ts, std::size_t... Is>
  auto
  print_args(const std::tuple<Ts...>& args, std::index_sequence<Is...>) const
      -> void
  {
    std::ignore = std::array<bool, sizeof...(Is)>{
        ((os_ << (Is == 0 ? "" : ", ") << std::get<Is>(args)), true)...};
  }

public:
  default_printer(std::ostream& os) : os_{os} {}

  template <class R>
  friend auto& operator<<(default_printer& p, const result<R>& r)
  {
    p.os_ << "test `" << r.name << "`...";

    if (r) {
      p.os_ << colors::pass << "[PASS]";
    } else {
      p.os_ << colors::fail << "[FAIL]";
    }
    p.os_ << colors::none;

    if (not r) {
      p.os_ << " " << r.source.file_name() << ":" << r.source.line() << "\n";

      constexpr auto N =
          std::tuple_size_v<detail::remove_cvref_t<decltype(r.arguments())>>;
      if (N) {
        p.os_ << r.pred_name() << "{}(";
        p.print_args(r.arguments(), std::make_index_sequence<N>{});
        p.os_ << ")";
      }

      p.os_ << "\n";
    }

    p.os_ << "\n";

    return p;
  }

  friend auto& operator<<(default_printer& p, const summary& s)
  {
    if (s.fail != 0) {
      p.os_ << tests{s.pass} << " passed | " << colors::fail << tests{s.fail}
            << " failed" << colors::none;
    } else {
      p.os_ << colors::pass << "all tests passed" << colors::none << " ("
            << tests{s.pass} << ")";
    }

    p.os_ << "\n";

    return p;
  }
};

template <class Printer>
class runner
{
  Printer printer_;
  summary summary_{};

public:
  template <
      class P = Printer,
      class = std::enable_if_t<std::is_default_constructible_v<P>>>
  runner() : printer_{}
  {}

  explicit runner(Printer p) : printer_{p} {}

  ~runner()
  {
    printer_ << summary_;
    std::exit(summary_.fail != 0);
  }

  template <class R>
  auto report(const result<R>& exp) & -> void
  {
    printer_ << exp;
    ++(exp ? summary_.pass : summary_.fail);
  }
};

struct override
{};

template <class = override>
auto cfg = runner<default_printer>{std::cout};

namespace detail {

template <class R>
auto is_result_(result<R>) -> std::true_type;
template <class T>
auto is_result_(const T&) -> std::false_type;

template <class T>
constexpr auto is_result_v =
    (not std::is_reference_v<T>)and decltype(is_result_(
        std::declval<T>()))::value;

class test
{
  std::string_view name_;

public:
  constexpr explicit test(std::string_view name) : name_{name} {}

  template <
      class F,
      class Override = std::enable_if_t<
          is_result_v<decltype(std::declval<const F&>()())>,
          override>>
  constexpr auto operator=(const F& func) -> void
  {
    auto result = func();
    result.name = name_;

    cfg<Override>.report(result);
  }
};

}  // namespace detail

namespace literals {
constexpr auto operator""_test(const char* name, std::size_t)
{
  return detail::test{name};
}
}  // namespace literals
}  // namespace skytest
