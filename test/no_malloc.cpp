#include <cstddef>
#include <cstdlib>
#include <iostream>

extern "C" {
// don't warn that malloc has a different parameter name with leading
// underscores
// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
auto malloc(std::size_t sz) -> void*
{
  std::cerr << "malloc called with " << sz << "\n";
  std::abort();
}
}
