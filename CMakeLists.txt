cmake_minimum_required(VERSION 3.28.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# OPTIONAL: Use ccache if present
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  message(STATUS "ccache found: ${CCACHE_PROGRAM}")
else()
  message(WARNING "ccache not found - builds will be slower. Install with: brew install ccache")
endif()

# Only brew LLVM is supported on OSX at the moment
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  # Find brew
  find_program(BREW_PROGRAM brew)
  if(BREW_PROGRAM)
    message(FATAL_ERROR "Homebrew not found! Install from https://brew.sh")
  endif()

  # Find LLVM
  execute_process(COMMAND brew --prefix llvm OUTPUT_VARIABLE LLVM_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE BREW_LLVM_RESULT ERROR_QUIET)
  if(NOT BREW_LLVM_RESULT EQUAL 0)
    message(FATAL_ERROR "Brew LLVM not found! Install with: brew install llvm")
  endif()

  # Use brew LLVM clang
  set(CMAKE_C_COMPILER "${LLVM_PREFIX}/bin/clang")
  set(CMAKE_CXX_COMPILER "${LLVM_PREFIX}/bin/clang++")

  # Link with brew LLVM libc++
  execute_process(COMMAND brew --prefix llvm OUTPUT_VARIABLE LLVM_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++ -L${LLVM_PREFIX}/lib/c++ -Wl,-rpath,${LLVM_PREFIX}/lib/c++)

  # Set OSX SDK
  execute_process(COMMAND xcrun --sdk macosx --show-sdk-path OUTPUT_VARIABLE OSX_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(CMAKE_OSX_SYSROOT "${OSX_SYSROOT}")
endif()

project(caesar VERSION 0.1.0)

# Optional: Enable code coverage
option(ENABLE_COVERAGE "Enable code coverage" OFF)

add_subdirectory(src/cmd)
add_subdirectory(src/core)

# Add coverage flags if enabled
if(ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang
    set(COVERAGE_COMPILE_FLAGS -fprofile-instr-generate -fcoverage-mapping)
    set(COVERAGE_LINK_FLAGS -fprofile-instr-generate)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # GCC
    set(COVERAGE_COMPILE_FLAGS --coverage)
    set(COVERAGE_LINK_FLAGS --coverage)
  endif()

  # Apply coverage flags to libraries
  target_compile_options(caesar_cmd PUBLIC ${COVERAGE_COMPILE_FLAGS})
  target_link_options(caesar_cmd PUBLIC ${COVERAGE_LINK_FLAGS})
  target_compile_options(caesar_core PUBLIC ${COVERAGE_COMPILE_FLAGS})
  target_link_options(caesar_core PUBLIC ${COVERAGE_LINK_FLAGS})

  message(STATUS "Code coverage enabled")
endif()

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
      test/cmd/test_environment.cpp
      test/cmd/test_interpreter.cpp
      test/cmd/test_object.cpp
      test/cmd/test_parser.cpp
      test/cmd/test_scanner.cpp
  )

  target_link_libraries(caesar_test PRIVATE caesar_cmd caesar_core Catch2::Catch2WithMain)

  # Apply coverage flags to test executable if coverage is enabled
  if(ENABLE_COVERAGE)
    target_compile_options(caesar_test PUBLIC ${COVERAGE_COMPILE_FLAGS})
    target_link_options(caesar_test PUBLIC ${COVERAGE_LINK_FLAGS})
  endif()

  include(CTest)
  include(Catch)
  catch_discover_tests(caesar_test)
  enable_testing()
endif()
