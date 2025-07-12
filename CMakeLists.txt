cmake_minimum_required(VERSION 3.10)
project(mystd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Find and include googletest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(include)
add_subdirectory(test)