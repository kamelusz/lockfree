#ifndef LF_TOOLS_PERF_COMMON_HEADER
#define LF_TOOLS_PERF_COMMON_HEADER

#include "fwd.h"

#include <vector>

using XaddPerf = lockfree::ring::GenericRing<void, 1024>;

enum class State {
  run,
  wait,
  terminate,
};

struct params {
  volatile State prod_state{State::wait};
  volatile State cons_state{State::wait};
  std::vector<producer<XaddPerf> *> producers;
};

#endif // LF_TOOLS_PERF_COMMON_HEADER
