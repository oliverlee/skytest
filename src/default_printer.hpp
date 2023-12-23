#pragma once

#include "src/result.hpp"
#include "src/utility.hpp"

#include <cstddef>
#include <ostream>
#include <string_view>
#include <type_traits>

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

  template <class Relation>
  auto& print(std::true_type, const Relation& r) &
  {
    const auto os = [&os = this->os_, &r]() -> auto& {
      os << (r ? colors::pass : colors::fail) << colors::dim;
      return os;
    };

    os() << "(";
    (*this) << std::get<0>(r.args);

    os() << " " << Relation::predicate_type::symbol << " ";
    (*this) << std::get<1>(r.args);

    os() << ")" << colors::none;

    return *this;
  }
  template <class Relation>
  auto& print(std::false_type, const Relation& r) &
  {
    os_ << (r ? colors::pass : colors::fail) << colors::dim << r
        << colors::none;

    return *this;
  }

  template <class F>
  static auto
  display(std::ostream& os, const F& msg) -> decltype(msg(os), void())
  {
    msg(os);
  }
  static auto display(std::ostream& os, std::string_view msg) -> void
  {
    os << msg;
  }

public:
  default_printer(std::ostream& os) : os_{os} {}

  template <class Relation>
  friend auto operator<<(default_printer& p, const Relation& r)
      -> decltype(std::declval<typename Relation::predicate_type>(), p)
  {
    static constexpr auto infix = std::is_same_v<
        notation::infix,
        typename Relation::predicate_type::notation_type>;

    static constexpr auto color_args =
        Relation::predicate_type::symbol == "and";

    return p.print(std::bool_constant < infix and color_args > {}, r);
  }

  friend auto& operator<<(default_printer& p, bool b)
  {
    p.print(std::false_type{}, b);
    return p;
  }

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
      p << r.relation;
      p.os_ << "\n";
      display(p.os_, r.msg);
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
