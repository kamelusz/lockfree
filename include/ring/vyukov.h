#ifndef LF_VYUKOV_RING_HEADER
#define LF_VYUKOV_RING_HEADER

#include <atomic>
#include <vector>
#include <memory>
#include <cstdint>

namespace lockfree {
namespace ring {
  
template <typename T, typename D>
class VyukovRing {
public:
  VyukovRing();
  ~VyukovRing() = default;

  decltype(auto) enqueue(T *data);
  T *dequeue();

private:
  struct entry {
    std::atomic<std::size_t> sequence_{0};
    T *data_{nullptr};
  };

  alignas(cpu::CACHE_SIZE) std::atomic<std::size_t> enqueue_pos_{};
  alignas(cpu::CACHE_SIZE) std::atomic<std::size_t> dequeue_pos_{};
  alignas(cpu::CACHE_SIZE) std::unique_ptr<entry[]> array_{};
};

template <typename T, typename D>
VyukovRing<T, D>::VyukovRing() {
  const D *derived = static_cast<D *>(this);
  constexpr auto size = derived->size();

  array_ = std::make_unique<entry[]>(size);

  for (std::size_t i = 0; i < size; ++i)
    array_[i].sequence_.store(i);
}

template <typename T, typename D>
decltype(auto) VyukovRing<T, D>::enqueue(T *data) {
  const D *derived = static_cast<D *>(this);
  constexpr auto mask = derived->mask();
  entry *entity;
  std::size_t pos = enqueue_pos_.load(std::memory_order_relaxed);

  for (;;) {
    entity = &array_[pos & mask];
    std::size_t seq = entity->sequence_.load(std::memory_order_acquire);
    std::intptr_t dif =
        static_cast<std::intptr_t>(seq) - static_cast<intptr_t>(pos);

    if (dif == 0) {
      if (enqueue_pos_.compare_exchange_weak(pos, pos + 1,
                                             std::memory_order_relaxed))
        break;
    } else if (dif < 0)
      return D::Error::no_space;
    else
      pos = enqueue_pos_.load(std::memory_order_relaxed);
  }

  entity->data_ = data;
  entity->sequence_.store(pos + 1, std::memory_order_release);

  return D::Error::ok;
}

template <typename T, typename D>
T *VyukovRing<T, D>::dequeue() {
  const D *derived = static_cast<D *>(this);
  constexpr auto mask = derived->mask();
  entry *entity;
  size_t pos = dequeue_pos_.load(std::memory_order_relaxed);

  for (;;) {
    entity = &array_[pos & mask];
    std::size_t seq = entity->sequence_.load(std::memory_order_acquire);
    std::intptr_t dif =
        static_cast<std::intptr_t>(seq) - static_cast<intptr_t>(pos + 1);

    if (dif == 0) {
      if (dequeue_pos_.compare_exchange_weak(pos, pos + 1,
                                             std::memory_order_relaxed))
        break;
    } else if (dif < 0)
      return nullptr;
    else
      pos = dequeue_pos_.load(std::memory_order_relaxed);
  }

  T *data = entity->data_;
  entity->sequence_.store(pos + mask + 1, std::memory_order_release);

  return data;
}

} // namespace ring
} // namespace lockfree

#endif // LF_VYUKOV_RING_HEADER

