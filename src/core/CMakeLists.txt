add_library(caesar_core STATIC
    util.hpp
    dwarf/context.cpp
    dwarf/context.hpp
    dwarf/alloc.hpp
        context.hpp
        target.hpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_sources(caesar_core PRIVATE
        macho.hpp
        macho.cpp
    )
endif()

target_include_directories(caesar_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

find_package(libdwarf CONFIG REQUIRED)
target_link_libraries(caesar_core PUBLIC libdwarf::dwarf)
