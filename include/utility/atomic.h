#ifndef LF_UTILITY_ATOMIC_HEADER
#define LF_UTILITY_ATOMIC_HEADER

#include "utility/traits.h"

namespace lockfree {
namespace atomic {

// let SFINAE be \m/
template <typename T>
inline std::enable_if_t<traits::same_size<T, 1>::value, T> 
xadd(T *ptr, int arg) {
  // TODO Can we avoid cast?
  T result = static_cast<T>(arg);

  asm volatile("\n\tlock; xaddb %b0, %1\n"
               : "+q"(result), "+m"(*ptr)
               :
               : "memory", "cc");

  return result;
}

template <typename T>
inline std::enable_if_t<traits::same_size<T, 2>::value, T> 
xadd(T *ptr, int arg) {
  // TODO Can we avoid cast?
  T result = static_cast<T>(arg);

  asm volatile("\n\tlock; xaddw %w0, %1\n"
               : "+r"(result), "+m"(*ptr)
               :
               : "memory", "cc");

  return result;
}

template <typename T>
inline std::enable_if_t<traits::same_size<T, 4>::value, T> 
xadd(T *ptr, int arg) {
  // TODO Can we avoid cast?
  T result = static_cast<T>(arg);

  asm volatile("\n\tlock; xaddl %0, %1\n"
               : "+r"(result), "+m"(*ptr)
               :
               : "memory", "cc");

  return result;
}

template <typename T>
inline std::enable_if_t<traits::same_size<T, 8>::value, T> 
xadd(T *ptr, int arg) {
  // TODO Can we avoid cast?
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

#endif // LF_UTILITY_ATOMIC_HEADER
