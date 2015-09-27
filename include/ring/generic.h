#ifndef LF_GENERIC_RING_HEADER
#define LF_GENERIC_RING_HEADER

#include "xadd.h"

#include <functional>
#include <fstream>

namespace lockfree {
namespace ring {

template <typename T, std::size_t SIZE,
          template <typename, typename> class RingPolicy>
class GenericRing : private RingPolicy<T, GenericRing<T, SIZE, RingPolicy>> {
public:
  using RingPolicyBase = RingPolicy<T, GenericRing>;

  enum class Error {
    ok,
    no_space,
  };

  constexpr GenericRing(size_t producers)
      : producers_{producers}
      , ring_policy_{static_cast<RingPolicyBase *>(this)} {}
  ~GenericRing() = default;

  decltype(auto) enqueue(T *data) { return ring_policy_->enqueue(data); }
  T *dequeue() { return ring_policy_->dequeue(); }

  static constexpr std::size_t size() { return size_; }
  static constexpr std::size_t mask() { return mask_; }
  std::size_t producers() const { return producers_; }

  friend RingPolicyBase;

private:
  static constexpr std::size_t size_{SIZE};
  static constexpr std::size_t mask_{SIZE - 1};
  const std::size_t producers_;
  RingPolicyBase *ring_policy_;
};
} // namespace ring
} // namespace lockfree

#endif // LF_GENERIC_RING_HEADER
