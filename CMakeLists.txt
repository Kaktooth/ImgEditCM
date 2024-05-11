# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.21)
SET(CMAKE_CXX_STANDARD 20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_CXX_FLAGS "-O3 -mavx -msse3 -mavx2 -g -std=c++20 -Wall -Wextra -v")
endif()

project ("convolution_matrix")
include(FetchContent)
# Include sub-projects.
add_subdirectory ("convolution_matrix")

 