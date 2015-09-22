find_package(
  Threads
  )

add_executable(perf
  perf.cpp
  )

target_link_libraries (perf 
  ${CMAKE_THREAD_LIBS_INIT}
  )
