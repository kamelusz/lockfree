#ifndef LF_TOOLS_PERF_PRODUCER_HEADER
#define LF_TOOLS_PERF_PRODUCER_HEADER

#include "common.h"

#include <cstdint>
#include <xmmintrin.h>

template <typename T>
class producer {
public:
  producer(T &r) : ring_(r) {}

  void operator()(const params &param, std::uint8_t num);

  std::uint64_t writes() const { return write_count_; }
  std::uint64_t pauses() const { return pause_count_; }

private:
  std::uint64_t write_count_{};
  std::uint64_t pause_count_{};
  T &ring_;
};

template <typename T>
void producer<T>::operator()(const params &param, std::uint8_t num) {
  // set_cpu(num);

  while (param.prod_state == State::wait)
    _mm_pause();

  while (param.prod_state == State::run) {
    std::uint64_t data = num;
    data <<= 32;
    data += write_count_;

    auto result = ring_.enqueue(reinterpret_cast<void *>(data));
    switch (result) {
    case T::Error::no_space:
      ++pause_count_;
      _mm_pause();
      break;

    case T::Error::ok:
      ++write_count_;
      break;

    default:
      // TODO specify exception
      throw;
    }
  }
}

#endif // LF_TOOLS_PERF_PRODUCER_HEADER
