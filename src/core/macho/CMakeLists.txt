add_library(caesar_macho STATIC
    macho.cpp
    macho.hpp
    ports.hpp
    mach_exc.h
    mach_excServer.h
)

target_include_directories(caesar_macho PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_include_directories(caesar_macho PRIVATE
    $<TARGET_PROPERTY:caesar_core,INTERFACE_INCLUDE_DIRECTORIES>
)
