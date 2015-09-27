#include "producer.h"
#include "consumer.h"
#include "ring/generic.h"
#include "ring/xadd.h"
#include "ring/vyukov.h"

#include <iostream>
#include <algorithm>
#include <vector>

#include <thread>
#include <chrono>

#include <xmmintrin.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "USAGE: perf [producers-count]" << std::endl;
    exit(EXIT_SUCCESS);
  }

  int cons_num = std::stoi(argv[1]);

  XaddPerf ring(cons_num);

  params param{};

  std::vector<producer<XaddPerf>> producers;
  for (int i = 0; i < cons_num; ++i) {
    producers.push_back(producer<XaddPerf>{ring});
  }

  std::vector<std::thread> producer_threads;
  for (int i = 0; i < cons_num; ++i) {
    auto call = &producer<XaddPerf>::operator();
    std::thread prod_thread{call, &producers[i], std::cref(param), i};
    param.producers.push_back(&producers[i]);
    producer_threads.push_back(std::move(prod_thread));
  }

  param.cons_state = State::run;
  consumer<decltype(ring)> cons{ring, param};
  auto call = &consumer<XaddPerf>::operator();
  std::thread cons_thread{call, &cons, std::cref(param), cons_num};

  auto report = [&param, &producers, &cons]() {
    using namespace std::literals;

    std::this_thread::sleep_for(10s);

    std::uint64_t writes = 0;
    std::uint64_t pauses = 0;

    std::for_each(std::begin(producers), std::end(producers),
                  [&writes, &pauses](const auto &t) {
                    writes += t.writes();
                    pauses += t.pauses();
                  });

    std::cout << "writes " << writes << " ";
    std::cout << "pauses " << pauses << std::endl;
  };

  param.prod_state = State::run;
  report();
  param.prod_state = State::terminate;

  std::for_each(std::begin(producer_threads), std::end(producer_threads),
                [](auto &t) { t.join(); });

  ::sleep(1);
  param.cons_state = State::terminate;
  cons_thread.join();

  cons.is_consistent();

  return EXIT_SUCCESS;
}
