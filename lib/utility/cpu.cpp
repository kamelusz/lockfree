#include "utility/cpu.h"

#include <sched.h>
#include <sys/sysinfo.h>

#include <iostream>

namespace lockfree {
namespace cpu {

int set_cpu(unsigned long cpu) {
  int rc = 0;
  cpu_set_t set;

  CPU_ZERO(&set);
  CPU_SET(cpu, &set);

  rc = sched_setaffinity(0, sizeof(cpu_set_t), &set);
  if (rc == -1) {
    std::cerr << "sched_setaffinity" << std::endl;

    // TODO specify exception
    throw;
  }

  return rc;
}

} // namespace cpu
} // namespace lockfree
