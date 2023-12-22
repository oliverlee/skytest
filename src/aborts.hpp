#pragma once

#include <cstdlib>
#include <cstring>
#include <optional>
#include <ostream>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

namespace skytest {

inline constexpr struct
{
  struct return_type
  {
    std::optional<int> status{};

    constexpr explicit operator bool() const
    {
      return status and WIFSIGNALED(*status) and (WTERMSIG(*status) == SIGABRT);
    }

    friend auto& operator<<(std::ostream& os, const return_type& r)
    {
      if (not r.status) {
        os << "unable to fork process";
      } else {
        if (WIFEXITED(*r.status)) {
          os << "exited: " << WEXITSTATUS(*r.status);
        }
        if (WIFSIGNALED(*r.status)) {
          os << "terminated due to signal: SIG"
             << sigabbrev_np(WTERMSIG(*r.status));
        }
      }
      return os;
    }
  };

  template <class F>
  constexpr auto operator()(F&& f) const -> return_type
  {
    const auto pid = ::fork();
    if (pid == -1) {
      return {};
    }

    if (pid == 0) {
      std::forward<F>(f)();
      std::_Exit(EXIT_SUCCESS);
    }

    auto status = int{};
    ::wait(&status);
    return {status};
  }
} aborts{};

}  // namespace skytest
