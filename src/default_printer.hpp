#pragma once

#include "src/detail/arg_fmt.hpp"
#include "src/detail/priority.hpp"
#include "src/result.hpp"
#include "src/utility.hpp"

#include <cstddef>
#include <ostream>
#include <string_view>
#include <type_traits>

namespace skytest {

class default_printer
{
  template <int N>
  using priority = detail::priority<N>;

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
    (*this) << detail::arg_fmt(std::get<0>(r.args));

    os() << " " << Relation::predicate_type::symbol << " ";
    (*this) << detail::arg_fmt(std::get<1>(r.args));

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

  auto stream_impl(priority<4>, const summary& s) -> void
  {
    if (s.fail != 0) {
      os_ << tests{s.pass} << " passed | " << colors::fail << tests{s.fail}
          << " failed" << colors::none;
    } else {
      os_ << colors::pass << "all tests passed" << colors::none << " ("
          << tests{s.pass} << ")";
    }

    os_ << "\n";
  }
  template <class R, class M>
  auto stream_impl(priority<3>, const result<R, M>& r) -> void
  {
    os_ << "test `" << r.name << "`...";

    [&os = os_, runtime = r, compile_time = r.compile_time] {
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
    os_ << colors::none;

    if (not r) {
      os_ << " " << r.source.file_name() << ":" << r.source.line() << "\n";
      *this << r.relation;
      os_ << "\n";
      display(os_, r.msg);
      os_ << "\n";
    }

    os_ << "\n";
  }
  template <class Relation>
  auto stream_impl(priority<2>, const Relation& r)
      -> decltype(std::declval<typename Relation::predicate_type>(), void())
  {
    static constexpr auto infix = std::is_same_v<
        notation::infix,
        typename Relation::predicate_type::notation_type>;

    static constexpr auto color_args =
        Relation::predicate_type::symbol == "and";

    print(std::bool_constant < infix and color_args > {}, r);
  }
  auto stream_impl(priority<1>, bool b) -> void { print(std::false_type{}, b); }
  template <class T>
  auto stream_impl(priority<0>, const T& t) -> void
  {
    os_ << t;
  }

public:
  default_printer(std::ostream& os) : os_{os} {}

  template <class T>
  friend auto& operator<<(default_printer& p, const T& t)
  {
    p.stream_impl(detail::priority<4>{}, t);
    return p;
  }
};

}  // namespace skytest
