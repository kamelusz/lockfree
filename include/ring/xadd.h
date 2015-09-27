#ifndef LF_XADD_RING_HEADER
#define LF_XADD_RING_HEADER

#include "utility/atomic.h"
#include "utility/cpu.h"

#include <vector>

namespace lockfree {
namespace ring {

template <typename T, typename D>
class XaddRing {
public:
  XaddRing();
  ~XaddRing() = default;

  decltype(auto) enqueue(T *data);
  T *dequeue();

private:
  struct entry {
    T *buf{nullptr};
    std::uint64_t flag{0};
  };

  alignas(cpu::CACHE_SIZE) volatile std::size_t prod_head_;
  alignas(cpu::CACHE_SIZE) volatile std::size_t cons_head_;
  alignas(cpu::CACHE_SIZE) std::vector<entry> array_;
};

template <typename T, typename D>
XaddRing<T, D>::XaddRing()
    : prod_head_(0), cons_head_(0) {
  const D *derived = static_cast<D *>(this);

  array_.resize(derived->size());
}

template <typename T, typename D>
decltype(auto) XaddRing<T, D>::enqueue(T *data) {
  const D *derived = static_cast<D *>(this);
  constexpr auto mask = derived->mask();

  const auto space = (mask + cons_head_ - prod_head_) & mask;
  if (space < derived->producers())
    return D::Error::no_space;

  const auto old_head = atomic::xadd(&prod_head_, 1);
  const auto valid_head = old_head & mask;

  array_[valid_head].buf = data;
  lockfree::atomic::sfence();
  array_[valid_head].flag = 1;

  return D::Error::ok;
}

template <typename T, typename D>
T *XaddRing<T, D>::dequeue() {
  const D *derived = static_cast<D *>(this);
  constexpr auto mask = derived->mask();
  const auto head = cons_head_;

  atomic::lfence();
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
