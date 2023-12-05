#pragma once

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

namespace skytest {

template <class Relation>
class expectation;

namespace detail {
class source_location
{
  std::string_view file_{"unknown"};
  int line_{};

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
  source_location source;
  Relation rel;

  constexpr explicit operator bool() const { return bool(rel); }

  constexpr auto to_expectation(std::string_view name) &&
  {
    return expectation<Relation>{name, std::move(*this)};
  }
  constexpr friend auto operator|(std::string_view name, result&& result)
  {
    return std::move(result).to_expectation(name);
  }
};

template <class... Args>
struct relation
{
  std::tuple<const Args&...> args;
  bool value;

  constexpr operator bool() const { return value; }
};

template <class F>
struct predicate
{
  template <class... Ts>
  constexpr auto operator()(const Ts&... args) const -> std::remove_reference_t<
      decltype(bool(F{}(args...)), std::declval<relation<Ts...>>())>
  {
    return {std::tuple<const Ts&...>{args...}, F{}(args...)};
  }
};
}  // namespace detail

template <class Relation>
constexpr auto expect(
    Relation r, detail::source_location sl = detail::source_location::current())
{
  return detail::result<Relation>{sl, r};
}

template <class Relation>
class expectation
{
  std::string_view name_;
  detail::result<Relation> result_;

  friend struct ::skytest::detail::result<Relation>;

  constexpr expectation(std::string_view n, detail::result<Relation>&& r)
      : name_{n}, result_{std::move(r)}
  {}

public:
  constexpr auto name() const noexcept { return name_; }
  constexpr auto file() const noexcept { return result_.source.file_name(); }
  constexpr auto line() const noexcept { return result_.source.line(); }
  constexpr explicit operator bool() const { return bool(result_); }
};

inline constexpr auto eq = detail::predicate<std::equal_to<>>{};
inline constexpr auto ne = detail::predicate<std::not_equal_to<>>{};
inline constexpr auto lt = detail::predicate<std::less<>>{};
inline constexpr auto gt = detail::predicate<std::greater<>>{};
inline constexpr auto le = detail::predicate<std::less_equal<>>{};
inline constexpr auto ge = detail::predicate<std::greater_equal<>>{};

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

public:
  default_printer(std::ostream& os) : os_{os} {}

  template <class R>
  friend auto& operator<<(default_printer& p, const expectation<R>& exp)
  {
    p.os_ << "test `" << exp.name() << "`...";

    if (exp) {
      p.os_ << colors::pass << "[PASS]" << colors::none;
    } else {
      p.os_ << colors::fail << "[FAIL] " << colors::none << exp.file() << ":"
            << exp.line();
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
  auto report(const expectation<R>& exp) & -> void
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
auto is_expect_result_(result<R>) -> std::true_type;
template <class T>
auto is_expect_result_(const T&) -> std::false_type;

template <class T>
constexpr auto is_expect_result_v =
    (not std::is_reference<T>::value) and
    decltype(is_expect_result_(std::declval<T>()))::value;

class test
{
  std::string_view name_;

public:
  constexpr explicit test(std::string_view name) : name_{name} {}

  template <
      class F,
      class Override = std::enable_if_t<
          is_expect_result_v<std::result_of_t<const F&()>>,
          override>>
  constexpr auto operator=(const F& func) -> void
  {
    cfg<Override>.report(name_ | func());
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
