#ifndef LF_XADD_RING_HEADER
#define LF_XADD_RING_HEADER

#include "utility/cpu.h"

#include <atomic>
#include <memory>

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
    std::atomic<std::uint32_t> flag{0};
  };

  alignas(cpu::CACHE_SIZE) volatile std::atomic<std::size_t> prod_head_{};
  alignas(cpu::CACHE_SIZE) volatile std::size_t cons_head_{};
  alignas(cpu::CACHE_SIZE) std::unique_ptr<entry[]> array_{};
};

template <typename T, typename D>
XaddRing<T, D>::XaddRing() {
  const D *derived = static_cast<D *>(this);
  constexpr auto size = derived->size();

  array_ = std::make_unique<entry[]>(size);
}

template <typename T, typename D>
decltype(auto) XaddRing<T, D>::enqueue(T *data) {
  const D *derived = static_cast<D *>(this);
  constexpr auto mask = derived->mask();

  const auto space = (mask + cons_head_ - prod_head_) & mask;
  if (space < derived->producers())
    return D::Error::no_space;

  const auto old_head = prod_head_.fetch_add(1);
  const auto valid_head = old_head & mask;

  array_[valid_head].buf = data;
  array_[valid_head].flag.store(1, std::memory_order_release);

  return D::Error::ok;
}

template <typename T, typename D>
T *XaddRing<T, D>::dequeue() {
  const D *derived = static_cast<D *>(this);
  constexpr auto mask = derived->mask();
  const auto head = cons_head_;

  if (!(array_[head].flag.load(std::memory_order_acquire) & 1))
    return nullptr;

  T *ret = array_[head].buf;
  array_[head].flag.store(0, std::memory_order_release);
  cons_head_ = (head + 1) & mask;

  return ret;
}

} // namespace ring
} // namespace lockfree

#endif // LF_XADD_RING_HEADER

