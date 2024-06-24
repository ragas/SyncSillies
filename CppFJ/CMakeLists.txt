cmake_minimum_required(VERSION 3.27)
project(CPPFJ)
project(test)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 20)


message(NOTICE "Using build ==========" ${CMAKE_BUILD_TYPE} - ${CMAKE_SIZEOF_VOID_P}"==========")
include(FetchContent)
FetchContent_Declare(
  onetbb
  GIT_REPOSITORY https://github.com/oneapi-src/oneTBB
  GIT_TAG        v2021.12.0
)

FetchContent_MakeAvailable(onetbb)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(CPPFJ
  work.cpp
  main.cpp
)

add_executable(test
  work.cpp
  test.cpp
)

target_link_libraries(CPPFJ TBB::tbb)

target_link_libraries(test TBB::tbb Catch2::Catch2WithMain)
