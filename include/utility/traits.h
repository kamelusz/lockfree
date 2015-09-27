#ifndef LF_UTILITY_TRAITS_HEADER
#define LF_UTILITY_TRAITS_HEADER

#include <type_traits>
#include <cstddef>

namespace lockfree {
namespace traits {

template <typename T, std::size_t SIZE>
struct same_size : public std::integral_constant<bool, sizeof(T) == SIZE> {};

} // namespace traits
} // namespace lockfree

#endif // LF_UTILITY_TRAITS_HEADER
