#pragma once

#include <array>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#define SKYTEST_CXX20 (__cplusplus >= 202002L)

namespace skytest {

struct empty_symbol
{
  static constexpr auto value = std::string_view{};
};
struct notation
{
  struct function
  {};
  struct infix
  {};
};
template <class Symbol = empty_symbol, class Notation = notation::function>
struct predicate_format
{
  using notation_type = Notation;
  static constexpr auto symbol = Symbol::value;
};

namespace detail {

template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <class F>
constexpr auto predicate_type_name()
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

template <class F>
static constexpr auto type_name = predicate_type_name<F>();

template <class F, class G>
struct and_ : predicate_format<>
{
  static constexpr auto name = predicate_type_name<and_>();
};
template <class F, class G>
struct or_ : predicate_format<>
{
  static constexpr auto name = predicate_type_name<or_>();
};
template <class F>
struct not_ : predicate_format<>
{
  static constexpr auto name = predicate_type_name<not_>();
};

template <class... Ts>
struct tuple_fmt
{
  const std::tuple<Ts...>& value;

  template <std::size_t... Is>
  auto& stream_insert(std::index_sequence<Is...>, std::ostream& os) const
  {
    std::ignore = std::array<bool, sizeof...(Is)>{
        (os << (Is == 0 ? "" : ", ") << std::get<Is>(value), true)...};
    return os;
  }

  friend auto& operator<<(std::ostream& os, const tuple_fmt& tup)
  {
    return tup.stream_insert(std::index_sequence_for<Ts...>{}, os);
  }
};

template <class... Ts>
constexpr auto fmt(const std::tuple<Ts...>& tup)
{
  return tuple_fmt<Ts...>{tup};
}

template <class F, class... Ts>
struct relation
{
  using predicate_type = F;
  using arguments_type = std::tuple<Ts...>;

  arguments_type args;
  bool value;

  constexpr operator bool() const { return value; }

  static constexpr auto&
  print(notation::function, std::ostream& os, const relation& r)
  {
    if (const auto symbol = predicate_type::symbol; not symbol.empty()) {
      os << symbol;
    } else {
      os << predicate_type::name << "{}";
    }

    os << "(" << fmt(r.args) << ")";
    return os;
  }
  static auto& print(notation::infix, std::ostream& os, const relation& r)
  {
    static_assert(sizeof...(Ts) == 2);

    os << "(" << std::get<0>(r.args) << " " << predicate_type::symbol << " "
       << std::get<1>(r.args) << ")";
    return os;
  }

  friend auto& operator<<(std::ostream& os, const relation& r)
  {
    return print(typename relation::predicate_type::notation_type{}, os, r);
  }

  template <class G, class... Us>
  constexpr friend auto operator and(relation&& lhs, relation<G, Us...>&& rhs)
  {
    return relation<and_<F, G>, Ts..., Us...>{
        std::tuple_cat(std::move(lhs.args), std::move(rhs.args)),
        lhs.value and rhs.value};
  }
  template <class G, class... Us>
  constexpr friend auto operator or(relation&& lhs, relation<G, Us...>&& rhs)
  {
    return relation<or_<F, G>, Ts..., Us...>{
        std::tuple_cat(std::move(lhs.args), std::move(rhs.args)),
        lhs.value or rhs.value};
  }
  constexpr friend auto operator not(relation&& r)
  {
    return relation<not_<F>, Ts...>{std::move(r.args), not r.value};
  }
};

template <class F, const std::string_view& Name, class Format>
struct predicate : F
{
  static constexpr auto name = Name;
  using notation_type = typename Format::notation_type;
  static constexpr auto symbol = Format::symbol;

  template <
      class T = F,
      class = std::enable_if_t<std::is_default_constructible_v<T>>>
  constexpr predicate()
  {}

  template <class T, class = std::enable_if_t<std::is_constructible_v<F, T&&>>>
  constexpr predicate(T&& f) : F{std::forward<T>(f)}
  {}

  template <
      class... Ts,
      class =
          std::enable_if_t<std::is_invocable_r_v<bool, const F&, const Ts&...>>>
  constexpr auto operator()(Ts&&... args) const
  {
    const auto value = static_cast<const F&>(*this)(std::as_const(args)...);
    return relation<predicate, std::decay_t<Ts>...>{
        std::tuple<std::decay_t<Ts>...>{std::forward<Ts>(args)...}, value};
  }
};

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

template <class Relation, class Message>
struct result
{
  Relation relation;
  Message msg;
  source_location source;
  std::string_view name{"unknown"};
  std::optional<bool> compile_time{};

  constexpr explicit operator bool() const { return bool(relation); }
};

namespace detail {

template <class Format = predicate_format<>>
struct pred_fn
{
  template <class F>
  constexpr auto operator()(F&& f) const
  {
    return detail::predicate<detail::remove_cvref_t<F>, type_name<F>, Format>{
        std::forward<F>(f)};
  }
  template <class T, class F>
  constexpr auto operator()(T, F&& f) const
  {
    return pred_fn<T>{}(std::forward<F>(f));
  }
};
}  // namespace detail

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

namespace detail {
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

  template <class R, class M>
  auto report(const result<R, M>& r) & -> void
  {
    printer_ << r;
    ++(r ? summary_.pass : summary_.fail);
  }
};

struct override
{};

template <class = override>
auto cfg = runner<default_printer>{std::cout};

struct empty_message
{
  template <class T>
  constexpr auto operator()(T&) const -> void
  {}
};

template <class Relation, class Message = empty_message>
[[nodiscard]]
constexpr auto expect(
    Relation r,
    Message msg = {},
    source_location sl = source_location::current())
{
  return result<Relation, Message>{std::move(r), std::move(msg), sl};
}

namespace detail {

template <class R, class M>
auto is_result_(result<R, M>) -> std::true_type;
template <class T>
auto is_result_(const T&) -> std::false_type;

template <class T>
constexpr auto is_result_v =
    (not std::is_reference_v<T>)and decltype(is_result_(
        std::declval<T>()))::value;

template <class F, class = void>
struct compile_time_result
{
  static constexpr auto value = std::optional<bool>{};
};
template <class F>
struct compile_time_result<F, std::enable_if_t<bool{F{}()} or true>>
{
  static constexpr auto value = std::optional<bool>{bool{F{}()}};
};

class test
{
  std::string_view name_;

public:
  constexpr explicit test(std::string_view name) : name_{name} {}

  template <
      class F,
      class Override = std::enable_if_t<
          is_result_v<decltype(std::declval<const F&>()())>,
          override>,
      class Pass = compile_time_result<F>>
  constexpr auto operator=(const F& func) -> void
  {
    auto result = func();
    result.name = name_;
    result.compile_time = Pass::value;

    cfg<Override>.report(result);
  }
};

}  // namespace detail

namespace literals {
constexpr auto operator""_test(const char* name, std::size_t len)
{
  return detail::test{{name, len}};
}
}  // namespace literals
}  // namespace skytest
