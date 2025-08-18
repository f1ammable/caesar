cmake_minimum_required(VERSION 3.31.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(caesar VERSION 0.1.0)

set(CMD_SOURCES
    src/cmd/error.cpp
    src/cmd/scanner.cpp
    src/cmd/error.hpp
    src/cmd/scanner.hpp
    src/cmd/token.hpp
    src/cmd/util.cpp
    src/cmd/util.hpp
    src/cmd/expr.hpp
    src/cmd/astprint.cpp
    src/cmd/astprint.hpp
    src/cmd/parser.hpp
    src/cmd/parser.cpp
    src/cmd/parse_error.hpp
    src/cmd/interpreter.cpp
    src/cmd/interpreter.hpp
    src/core/macho.hpp
    src/core/macho.cpp)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer -frtti")

add_executable(caesar src/main.cpp ${CMD_SOURCES})
