add_library(caesar_cmd STATIC
    error.cpp
    error.hpp
    scanner.cpp
    scanner.hpp
    token.hpp
    util.cpp
    util.hpp
    expr.hpp
    parser.hpp
    parser.cpp
    interpreter.cpp
    interpreter.hpp
    stmnt.hpp
    environment.hpp
    environment.cpp
    callable.hpp
    stdlib.hpp
    object.hpp
    formatter.hpp
)

target_include_directories(caesar_cmd PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
