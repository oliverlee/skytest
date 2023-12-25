#include <unistd.h>

extern "C" {
auto fork() -> pid_t
{
  return -1;
}
}
