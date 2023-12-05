#pragma once

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace skytest {
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

struct expect
{
  source_location source;
  bool value;

  friend class expectation;
};

class test;
}  // namespace detail

constexpr auto expect(
    bool value, detail::source_location sl = detail::source_location::current())
{
  return detail::expect{sl, value};
}

class expectation
{
  std::string_view name_;
  detail::expect result_;

  friend class ::skytest::detail::test;

  constexpr expectation(std::string_view n, detail::expect r)
      : name_{n}, result_{r}
  {}

public:
  constexpr auto name() const noexcept { return name_; }
  constexpr auto file() const noexcept { return result_.source.file_name(); }
  constexpr auto line() const noexcept { return result_.source.line(); }
  constexpr explicit operator bool() const { return result_.value; }
};

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

  friend auto& operator<<(default_printer& p, const expectation& exp)
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

  auto report(expectation exp) & -> void
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
class test
{
  std::string_view name_;

public:
  constexpr explicit test(std::string_view name) : name_{name} {}

  template <
      class F,
      class Override = std::enable_if_t<
          std::is_same<detail::expect, std::result_of_t<const F&()>>::value,
          override>>
  constexpr auto operator=(const F& func) -> void
  {
    cfg<Override>.report({name_, func()});
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
