#ifndef LF_UTILITY_HEADER
#define LF_UTILITY_HEADER

#include <type_traits>
#include <cstdlib>
#include <cstdint>

namespace lockfree {
namespace atomic {

constexpr std::uint8_t byte_size = 1;
constexpr std::uint8_t word_size = 2;
constexpr std::uint8_t dword_size = 4;
constexpr std::uint8_t qword_size = 8;

// let SFINAE be \m/
template <typename T>
inline std::enable_if_t<sizeof(T) == dword_size, T> 
xadd(T *ptr, int arg) {
  T result = static_cast<T>(arg);

  asm volatile("\n\tlock; xaddl %0, %1\n"
               : "+r"(result), "+m"(*ptr)
               :
               : "memory", "cc");

  return result;
}

template <typename T>
inline std::enable_if_t<sizeof(T) == qword_size, T> 
xadd(T *ptr, int arg) {
  T result = static_cast<T>(arg);

  asm volatile("\n\tlock; xaddq %q0, %1\n"
               : "+r"(result), "+m"(*ptr)
               :
               : "memory", "cc");

  return result;
}

// TODO analize std barrier implementation
inline void mfence() { asm volatile("mfence" ::: "memory"); }
inline void lfence() { asm volatile("lfence" ::: "memory"); }
inline void sfence() { asm volatile("sfence" ::: "memory"); }

} // namespace atomic
} // namespace lockfree
#endif // LF_UTILITY_HEADER
