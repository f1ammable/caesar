#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

#include "cmd/object.hpp"
#include "expected.hpp"
#include "token_type.hpp"
#include "typedefs.hpp"

namespace detail {
bool isop(char c);
TokenType ctoop(char c);
double parseNumber(const std::string& str);
std::vector<std::string> convertToStr(const std::vector<Object>& vec);

// Object extraction helpers
template <typename T = u64>
inline Expected<T, std::string> asAddr(const Object& obj) {
  if (const auto* d = std::get_if<double>(&obj)) {
    return static_cast<T>(*d);
  }
  if (const auto* s = std::get_if<std::string>(&obj)) {
    try {
      if (s->starts_with("0x") || s->starts_with("0X")) {
        return static_cast<T>(std::stoull(*s, nullptr, 16));
      }
      return static_cast<T>(std::stoull(*s, nullptr, 10));
    } catch (...) {
      return Unexpected{std::format("Cannot parse '{}' as number", *s)};
    }
  }
  return Unexpected<std::string>{"Expected numeric value"};
}

inline Expected<std::string, std::string> asString(const Object& obj) {
  if (const auto* s = std::get_if<std::string>(&obj)) {
    return *s;
  }
  return Unexpected{"Expected string"};
}

}  // namespace detail

#endif
