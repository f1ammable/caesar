#ifndef UTIL_H
#define UTIL_H
#include "token.hpp"

namespace detail {
bool isop(char c);
TokenType ctoop(char c);
std::variant<int, float, double> parseNumber(const std::string& str);
}  // namespace detail

#endif
