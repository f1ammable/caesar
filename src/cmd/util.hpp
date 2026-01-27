#ifndef UTIL_H
#define UTIL_H

#include <string>

#include "token_type.hpp"

namespace detail {
bool isop(char c);
TokenType ctoop(char c);
double parseNumber(const std::string& str);
}  // namespace detail

#endif
