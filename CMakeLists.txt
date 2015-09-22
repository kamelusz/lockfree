cmake_minimum_required(VERSION 2.8)

project(lockfree)

if (UNIX)
  set(CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} -std=c++14 -pedantic -Wall -ggdb -O2"
    )
endif(UNIX)

set(HW_INCLUDE_DIR 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

include_directories(
  ${HW_INCLUDE_DIR}
  )

add_subdirectory(tools)
