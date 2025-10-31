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
    src/cmd/stmnt.hpp
    src/cmd/environment.hpp
    src/cmd/environment.cpp)

set(CORE_SOURCES src/core/macho.cpp src/core/macho.hpp src/core/util.hpp)
set(DWARF_SOURCES src/core/dwarf/context.cpp src/core/dwarf/context.hpp src/core/dwarf/alloc.hpp)
add_executable(caesar src/main.cpp ${CMD_SOURCES} ${CORE_SOURCES}
                      ${DWARF_SOURCES})

find_package(libdwarf CONFIG REQUIRED)
target_link_libraries(caesar PUBLIC libdwarf::dwarf)
