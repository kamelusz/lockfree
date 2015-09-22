#include "ring/generic.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdlib>

#include <thread>
#include <chrono>

#include <xmmintrin.h>
#include <unistd.h>
#include <sched.h>
#include <sys/sysinfo.h>

enum class State {
  run,
  wait,
  terminate
};

template <typename T>
class producer;

using RingType = lockfree::ring::GenericRing<void, 1024>;

struct params {
  volatile State prod_state {State::wait};
  volatile State cons_state {State::wait};
  std::vector<producer<RingType> *> producers;
};

//TODO move to separate file
int set_cpu(unsigned long cpu)
{
  int rc = 0;
  cpu_set_t set;

  CPU_ZERO(&set);
  CPU_SET(cpu, &set);

  rc = sched_setaffinity(0, sizeof(cpu_set_t), &set);
  if (rc == -1) {
    std::cout << "sched_setaffinity" << std::endl;
    
    //TODO specify exception
    throw;
  }

  return rc;
}

template <typename T>
class producer {
  public:
    producer(T &r) : ring_(r) {}

    void operator()(const params &param, std::uint8_t num) {
      set_cpu(num);

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
            //TODO specify exception
            throw;
        }
      }
    }

    std::uint64_t writes() const { return write_count_; }
    std::uint64_t pauses() const { return pause_count_; }

  private:
    std::uint64_t write_count_ {};
    std::uint64_t pause_count_ {};
    T &ring_;
};

template <typename T>
class consumer {
  public:
    consumer(T &r, const params &p) : ring_(r), param_(p)  {
      auto producers = ring_.producers();
      check_ = new uint32_t* [producers];

      for (decltype(producers) i = 0; i < producers; ++i)
        check_[i] = new uint32_t [300000000];
    }

    void operator()(const params &param, std::uint8_t num) {
      set_cpu(num);

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

    bool is_consistent() {
      bool result = true;

      for (std::size_t i = 0; i < ring_.producers(); ++i) {
        std::cout << "Consistency check for " << i << " producer... ";

        for (std::uint32_t j = 0; j < param_.producers[i]->writes(); ++j) {
          if (j != check_[i][j])
            std::cout << "failure at " << j << " (" << check_[i][j] << ")" << std::endl;
        }

        std::cout << "ok" << std::endl;
      }

      return result;
    }

  private:
    T &ring_;
    const params &param_;
    uint32_t **check_;
};

int main(int argc, char *argv[]) 
{
  if (argc != 2) {
    std::cout << "USAGE: perf [producers-count]" << std::endl;
    exit(EXIT_SUCCESS);
  }

  int cons_num = std::stoi(argv[1]);

  RingType ring(cons_num);

  params param {};

  std::vector<producer<RingType>> producers;
  for (int i = 0; i < cons_num; ++i) {
    producers.push_back(producer<RingType> {ring});
  }

  std::vector<std::thread> producer_threads;
  for (int i = 0; i < cons_num; ++i) {
    auto call = &producer<RingType>::operator();
    std::thread prod_thread {call, &producers[i], std::cref(param), i};
    param.producers.push_back(&producers[i]);
    producer_threads.push_back(std::move(prod_thread));
  }

  param.cons_state = State::run;
  consumer<decltype(ring)> cons {ring, param};
  auto call = &consumer<RingType>::operator();
  std::thread cons_thread {call, &cons, std::cref(param), cons_num};

  auto report = [&param, &producers, &cons]() {
    using namespace std::literals;

    std::this_thread::sleep_for(10s);

    std::uint64_t writes = 0;
    std::uint64_t pauses = 0;

    std::for_each(std::begin(producers), std::end(producers), 
        [&writes, &pauses](const auto &t) {
          writes += t.writes();
          pauses += t.pauses();
        }
    );

    std::cout << "writes " << writes << " ";
    std::cout << "pauses " << pauses << std::endl;
  };

  param.prod_state = State::run;
  report();
  param.prod_state = State::terminate;

  std::for_each(std::begin(producer_threads), std::end(producer_threads), 
      [](auto &t) { t.join(); }
  );

  ::sleep(1);
  param.cons_state = State::terminate;
  cons_thread.join();

  cons.is_consistent();

  return EXIT_SUCCESS;
}
