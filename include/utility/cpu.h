#ifndef LF_UTILITY_CPU_HEADER
#define LF_UTILITY_CPU_HEADER

#include <cstddef>

namespace lockfree {
namespace cpu {

// TODO detect cache line at compile time?
constexpr std::size_t CACHE_SIZE = 128;

int set_cpu(unsigned long cpu);

} // namespace cpu
} // namespace lockfree

#endif // LF_UTILITY_CPU_HEADER
