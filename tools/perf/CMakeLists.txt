set(PERF_INCLUDE_DIR 
  ${HW_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

include_directories(
  ${PERF_INCLUDE_DIR}
  )

find_package(
  Threads
  )

add_executable(perf
  main.cpp
  )

target_link_libraries (perf 
  ${CMAKE_THREAD_LIBS_INIT}
  )
