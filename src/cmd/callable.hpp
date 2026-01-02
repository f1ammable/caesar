#ifndef CALLABLE_H
#define CALLABLE_H

#include <concepts>
#include <format>
#include <vector>

#include "object.hpp"

class Interpreter;

class Callable {
 public:
  virtual ~Callable() = default;
  virtual Object call(std::vector<Object> args) = 0;
  virtual int arity() const = 0;
  [[nodiscard]] virtual std::string str() const = 0;
};

template <typename T>
  requires std::derived_from<T, Callable>
struct std::formatter<T> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const T& fn, std::format_context& ctx) const {
    return std::format_to(ctx.out(), fn.str());
  }
};

#endif
