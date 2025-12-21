cmake_minimum_required(VERSION 3.31.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(caesar VERSION 0.1.0)

# Optional: Enable code coverage
option(ENABLE_COVERAGE "Enable code coverage" OFF)

add_subdirectory(src/cmd)
add_subdirectory(src/core)

add_executable(caesar src/main.cpp)
target_link_libraries(caesar PRIVATE caesar_cmd caesar_core)

# Optional: Build tests
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR OR CAESAR_BUILD_TESTS)
  option(CAESAR_BUILD_TESTS "Build tests" ON)
else()
  option(CAESAR_BUILD_TESTS "Build tests" OFF)
endif()

if(CAESAR_BUILD_TESTS)
  find_package(Catch2 CONFIG REQUIRED)
  add_executable(
    caesar_test
      test/cmd/test_scanner.cpp
  )

  target_link_libraries(caesar_test PRIVATE caesar_cmd caesar_core Catch2::Catch2WithMain)

  include(CTest)
  include(Catch)
  enable_testing()
  catch_discover_tests(caesar_test)
endif()
