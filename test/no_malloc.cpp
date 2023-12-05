#include <cstddef>
#include <cstdlib>
#include <iostream>

extern "C" {
auto malloc(std::size_t sz) -> void*
{
  std::cerr << "malloc called with " << sz << "\n";
  std::abort();
}
}
