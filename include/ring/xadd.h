#ifndef LF_XADD_RING_HEADER
#define LF_XADD_RING_HEADER

#include "utility/atomic.h"

#include <xmmintrin.h>
#include <vector>

namespace lockfree {
namespace ring {

template <typename T, typename D>
class XaddRing {
public:
  XaddRing() : prod_head_(0), cons_head_(0) {
    const D *derived = static_cast<D *>(this);

    array_.resize(derived->size());
  }
  ~XaddRing() = default;

  decltype(auto) enqueue(T *data);
  T *dequeue();

private:
  struct entry {
    T *buf{nullptr};
    std::uint64_t flag{0};
  };

  volatile std::size_t prod_head_ __attribute((aligned(128)));
  volatile std::size_t cons_head_ __attribute((aligned(128)));
  std::vector<entry> array_ __attribute((aligned(128)));
};

template <typename T, typename D>
decltype(auto) XaddRing<T, D>::enqueue(T *data) {
  const D *derived = static_cast<D *>(this);
  constexpr std::uint32_t mask = derived->mask();
  const std::uint32_t space = (mask + cons_head_ - prod_head_) & mask;
  if (space < derived->producers())
    return D::Error::no_space;

  const std::uint32_t old_head = lockfree::atomic::xadd(&prod_head_, 1);
  const std::uint32_t valid_head = old_head & mask;

  array_[valid_head].buf = data;
  lockfree::atomic::sfence();
  array_[valid_head].flag = 1;

  return D::Error::ok;
}

template <typename T, typename D>
T *XaddRing<T, D>::dequeue() {
  const D *derived = static_cast<D *>(this);
  constexpr std::uint32_t mask = derived->mask();
  const std::uint32_t head = cons_head_;

  lockfree::atomic::lfence();
  if (!(array_[head].flag & 1))
    return nullptr;

  T *ret = array_[head].buf;
  array_[head].flag = 0;
  cons_head_ = (head + 1) & mask;

  return ret;
}
} // namespace ring
} // namespace lockfree

#endif // LF_XADD_RING_HEADER
