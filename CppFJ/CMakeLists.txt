cmake_minimum_required(VERSION 3.27)
project(CPPFJ)


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


add_executable(CPPFJ
  work.cpp
)
target_link_libraries(CPPFJ TBB::tbb)
