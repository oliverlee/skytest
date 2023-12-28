#pragma once

#include "src/cfg.hpp"
#include "src/optional.hpp"

#include <cstdlib>
#include <cstring>
#include <ostream>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

namespace skytest {

namespace detail {
struct aborts_fn
{
  struct return_type
  {
    optional<int> status{};

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

  struct do_flush
  {
    friend auto& operator<<(std::ostream& os, do_flush)
    {
      os.flush();
      return os;
    }
  };

  template <class F, class Override = override>
  constexpr auto operator()(F&& f) const -> return_type
  {
    cfg<Override>.printer_ << do_flush{};

    const auto pid = ::fork();
    if (pid == -1) {
      return {};
    }

    if (pid == 0) {
      cfg<Override>.silence();

      std::forward<F>(f)();

      // NOLINTNEXTLINE(concurrency-mt-unsafe)
      std::exit(EXIT_SUCCESS);
    }

    auto status = int{};
    ::wait(&status);
    return {status};
  }
};
}  // namespace detail
inline constexpr auto aborts = detail::aborts_fn{};

}  // namespace skytest
