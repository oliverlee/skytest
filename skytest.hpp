#pragma once

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace skytest {

struct expect_
{
  bool value;
};
constexpr auto expect(bool value)
{
  return expect_{value};
}

struct expect_result
{
  std::string_view name;
  bool value;

  constexpr explicit operator bool() const { return value; }
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

  friend auto& operator<<(default_printer& p, const expect_result& er)
  {
    p.os_ << "test `" << er.name << "`...";

    if (er) {
      p.os_ << colors::pass << "[PASS]";
    } else {
      p.os_ << colors::fail << "[FAIL]";
    }
    p.os_ << colors::none << "\n";

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
  summary summary_;

public:
  explicit runner(Printer p) : printer_{p} {}

  ~runner()
  {
    printer_ << summary_;
    std::exit(summary_.fail != 0);
  }

  auto report(expect_result er) & -> void
  {
    printer_ << er;
    ++(er ? summary_.pass : summary_.fail);
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
          std::is_same<expect_, std::result_of_t<const F&()>>::value,
          override>>
  constexpr auto operator=(const F& func) -> void
  {
    cfg<Override>.report({name_, func().value});
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
