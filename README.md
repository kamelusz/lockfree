# lockfree
The primary goal of the library is to gather non-blocking algorithms I used at work.

There are two requirements I try to meet in implementation:
 - perfomance;
 - usability;

Because of these requirements the following decisions were taken:
 - implement every algorithm as policy in terms of policy-based classes [1];
 - use curiously recurring template pattern to reduce class members [2];
 - use compile-time solutions heavily (templates and constant expressions) [3];

At this moment library contains only two array-based algorithms: 
 - the bounded MPMC queue proposed by Dmitry Vyukov [4];
 - the own bounded queue called XaddRing because of using instructions xadd initially;

TODO:
 - add more algorithms;
 - integrate google/benchmark [5];
 - move XaddRing from xadd to std::atomic;

[1] Modern C++ Design: Generic Programming and Design Patterns Applied by Andrei Alexandrescu <br>
[2] Curiously Recurring Template Patterns by James Coplien (C++ Report: 24–27, February 1995) <br>
[3] C++ FAQ at https://isocpp.org (https://isocpp.org/wiki/faq/cpp11-language#cpp11-constexpr) <br>
[4] http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue <br>
[5] There are usefull video from CppCon2015: Chandler Carruth "Tuning C++: Benchmarks, and CPUs, <br>
    and Compilers! Oh My!" -- http://www.youtube.com/watch?v=nXaxk27zwlk
