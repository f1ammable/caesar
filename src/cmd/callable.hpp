#ifndef CALLABLE_H
#define CALLABLE_H

#include <concepts>
#include <format>
#include <vector>

#include "core/context.hpp"
#include "core/target.hpp"
#include "object.hpp"

class Interpreter;

class Callable {
 protected:
  std::unique_ptr<Target>& m_target = Context::getTarget();

 public:
  virtual ~Callable() = default;
  virtual Object call(std::vector<Object> args) = 0;
  [[nodiscard]] virtual int arity() const = 0;
  [[nodiscard]] virtual std::string str() const = 0;
};

template <typename T>
  requires std::derived_from<T, Callable>
struct std::formatter<T> {  // NOLINT(cert-dcl58-cpp)
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const T& fn, std::format_context& ctx) const {
    return std::format_to(ctx.out(), fn.str());
  }
};

#endif
