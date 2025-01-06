cmake_minimum_required(VERSION 3.31.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 11.1.1
)
FetchContent_MakeAvailable(fmt)

project(caesar VERSION 0.1.0)
add_executable(caesar src/main.cpp src/syscall_decode.h src/syscall_decode.cpp)
target_link_libraries(caesar fmt::fmt)

