#ifndef LF_RING_FWD_HEADER
#define LF_RING_FWD_HEADER

#include <cstddef>

namespace lockfree {
namespace ring {

template <typename T, typename D>
class XaddRing;

template <typename T, std::size_t SIZE = 1024,
          template <typename, typename> class RingPolicy = XaddRing>
class GenericRing;
}
}

#endif // LF_RING_FWD_HEADER

