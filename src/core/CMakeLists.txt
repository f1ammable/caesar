add_library(caesar_core STATIC
    util.hpp
    dwarf/context.cpp
    dwarf/context.hpp
    dwarf/alloc.hpp
        context.hpp
        target.hpp
        target.cpp
        platform.hpp
        decoder.hpp
        decoder.cpp
        decoder_traits.hpp
)

target_include_directories(caesar_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

find_package(libdwarf CONFIG REQUIRED)
find_package(capstone CONFIG REQUIRED)
target_link_libraries(caesar_core PUBLIC libdwarf::dwarf PRIVATE capstone::capstone)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(macho)
endif()
