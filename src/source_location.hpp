#pragma once

#include <string_view>

namespace skytest {

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

  [[nodiscard]]
  constexpr auto file_name() const noexcept
  {
    return file_;
  }
  [[nodiscard]]
  constexpr auto line() const noexcept
  {
    return line_;
  }
};

}  // namespace skytest
