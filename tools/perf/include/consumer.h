#ifndef LF_TOOLS_PERF_CONSUMER_HEADER
#define LF_TOOLS_PERF_CONSUMER_HEADER

#include "common.h"

#include <iostream>
#include <xmmintrin.h>

template <typename T>
class consumer {
public:
  consumer(T &r, const params &p);

  void operator()(const params &param, std::uint8_t num);

  bool is_consistent();

private:
  T &ring_;
  const params &param_;
  std::uint32_t **check_;
};

template <typename T>
consumer<T>::consumer(T &r, const params &p)
    : ring_(r), param_(p) {
  auto producers = ring_.producers();
  check_ = new uint32_t *[producers];

  for (decltype(producers) i = 0; i < producers; ++i)
    check_[i] = new uint32_t[300000000];
}

template <typename T>
void consumer<T>::operator()(const params &param, std::uint8_t num) {
  // set_cpu(num);

  while (param.cons_state == State::wait)
    _mm_pause();

  while (param.cons_state == State::run) {
    auto result = ring_.dequeue();

    if (result == nullptr)
      continue;

    const std::uint64_t data = *reinterpret_cast<uint64_t *>(&result);
    const std::uint32_t producer = data >> 32;
    const std::uint32_t sequence = data & 0x00000000FFFFFFFF;

    check_[producer][sequence] = sequence;
  }
}

template <typename T>
bool consumer<T>::is_consistent() {
  bool result = true;

  for (std::size_t i = 0; i < ring_.producers(); ++i) {
    std::cout << "Consistency check for " << i << " producer... ";

    for (std::uint32_t j = 0; j < param_.producers[i]->writes(); ++j) {
      if (j != check_[i][j])
        std::cout << "failure at " << j << " (" << check_[i][j] << ")"
                  << std::endl;
    }

    std::cout << "ok" << std::endl;
  }

  return result;
}

#endif // LF_TOOLS_PERF_CONSUMER_HEADER
