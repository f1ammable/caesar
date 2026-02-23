#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

#include "cmd/object.hpp"
#include "token_type.hpp"

namespace detail {
bool isop(char c);
TokenType ctoop(char c);
double parseNumber(const std::string& str);
std::vector<std::string> convertToStr(const std::vector<Object>& vec);
}  // namespace detail

#endif
