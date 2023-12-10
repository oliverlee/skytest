#pragma once

#include "src/result.hpp"
#include "src/utility.hpp"

#include <cstddef>
#include <ostream>
#include <string_view>

namespace skytest {

class default_printer
{
  std::ostream& os_;

  struct colors
  {
    static constexpr auto none = std::string_view{"\033[0m"};
    static constexpr auto dim = std::string_view{"\033[2m"};
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

  template <class R, class M>
  friend auto& operator<<(default_printer& p, const result<R, M>& r)
  {
    p.os_ << "test `" << r.name << "`...";

    [&os = p.os_, runtime = r, compile_time = r.compile_time] {
      if (runtime and compile_time == true) {
        os << colors::pass << "[CONSTEXPR PASS]";
        return;
      }
      if (runtime and compile_time == std::nullopt) {
        os << colors::pass << "[PASS]";
        return;
      }
      if (runtime and compile_time == false) {
        os << colors::pass << "[PASS]" << colors::fail << colors::dim
           << "(CONSTEXPR FAIL)";
        return;
      }
      if (not runtime and compile_time == true) {
        os << colors::fail << "[FAIL]" << colors::pass << colors::dim
           << "(CONSTEXPR PASS)";
        return;
      }
      if (not runtime and compile_time == std::nullopt) {
        os << colors::fail << "[FAIL]";
        return;
      }
      if (not runtime and compile_time == false) {
        os << colors::fail << "[CONSTEXPR FAIL]";
        return;
      }
    }();
    p.os_ << colors::none;

    if (not r) {
      p.os_ << " " << r.source.file_name() << ":" << r.source.line() << "\n";
      p.os_ << colors::fail << colors::dim << r.relation << colors::none;
      r.msg(p.os_);
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

}  // namespace skytest
