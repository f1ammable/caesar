#ifndef OBJECT_H
#define OBJECT_H

#include <format>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

#include "runtime_error.hpp"

class Callable;

using Object = std::variant<std::monostate, std::string, float, int, double,
                            bool, std::shared_ptr<Callable>>;

template <typename Op>
inline Object binary_operation(const Object& lhs, const Object& rhs, Op op,
                               const std::string& op_name) {
  auto visitor = [&op, &op_name](const auto& left,
                                 const auto& right) -> Object {
    using L = std::decay_t<decltype(left)>;
    using R = std::decay_t<decltype(right)>;

    if constexpr (std::is_arithmetic_v<L> && std::is_arithmetic_v<R>) {
      return op(static_cast<double>(left), static_cast<double>(right));
    } else if constexpr (std::is_same_v<L, std::string> &&
                         std::is_same_v<R, std::string>) {
      if constexpr (std::is_same_v<Op, std::plus<>>) {
        return left + right;
      } else {
        throw RuntimeError(
            std::format("Cannot apply operator {} to strings", op_name));
      }
    } else {
      throw RuntimeError(
          std::format("Unsupported operand types for {}", op_name));
    }
  };

  return std::visit(visitor, lhs, rhs);
}

template <typename Op>
inline bool comparison_operation(const Object& lhs, const Object& rhs, Op op,
                                 const std::string& op_name) {
  auto visitor = [&op, &op_name](const auto& left, const auto& right) -> bool {
    using L = std::decay_t<decltype(left)>;
    using R = std::decay_t<decltype(right)>;

    if constexpr (std::is_arithmetic_v<L> && std::is_arithmetic_v<R>) {
      return op(static_cast<double>(left), static_cast<double>(right));
    } else {
      throw RuntimeError(std::format(
          "Cannot apply operator {} to non-arithmetic types", op_name));
    }
  };

  return std::visit(visitor, lhs, rhs);
}

inline Object operator+(const Object& lhs, const Object& rhs) {
  auto visitor = [](const auto& left, const auto& right) -> Object {
    using L = std::decay_t<decltype(left)>;
    using R = std::decay_t<decltype(right)>;

    if constexpr (std::is_same_v<L, std::string> &&
                  std::is_same_v<R, std::string>) {
      return left + right;
    }

    return binary_operation(left, right, std::plus<double>{}, "+");
  };

  return std::visit(visitor, lhs, rhs);
}

inline Object operator-(const Object& lhs, const Object& rhs) {
  return binary_operation(lhs, rhs, std::minus<double>{}, "-");
}

inline Object operator/(const Object& lhs, const Object& rhs) {
  return binary_operation(lhs, rhs, std::divides<double>{}, "/");
}

inline Object operator*(const Object& lhs, const Object& rhs) {
  return binary_operation(lhs, rhs, std::multiplies<double>{}, "*");
}

inline bool operator==(const Object& lhs, const Object& rhs) {
  return comparison_operation(lhs, rhs, std::equal_to<double>{}, "==");
}

inline bool operator!=(const Object& lhs, const Object& rhs) {
  return comparison_operation(lhs, rhs, std::not_equal_to<double>{}, "!=");
}

inline bool operator<(const Object& lhs, const Object& rhs) {
  return comparison_operation(lhs, rhs, std::less<double>{}, "<");
}

inline bool operator>(const Object& lhs, const Object& rhs) {
  return comparison_operation(lhs, rhs, std::greater<double>{}, ">");
}

inline bool operator<=(const Object& lhs, const Object& rhs) {
  return comparison_operation(lhs, rhs, std::less_equal<double>{}, "<=");
}

inline bool operator>=(const Object& lhs, const Object& rhs) {
  return comparison_operation(lhs, rhs, std::greater_equal<double>{}, ">=");
}

namespace detail {
template <typename T>
  requires(std::is_arithmetic_v<T>)
[[nodiscard]] std::string to_string(const T& val, const int p = 3) {
  std::ostringstream out;
  out.precision(p);
  out << std::fixed << val;
  return std::move(out).str();
}

inline std::string bool_to_s(bool b) { return b ? "(true)" : "(false)"; }
}  // namespace detail

template <>
struct std::formatter<Object> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const Object& obj, std::format_context& ctx) const {
    auto visitor = [&ctx](const auto& value) -> std::format_context::iterator {
      using T = std::decay_t<decltype(value)>;

      if constexpr (std::is_same_v<T, std::monostate>) {
        return std::format_to(ctx.out(), "emtpy");
      } else if constexpr (std::is_same_v<T, std::string>) {
        return std::format_to(ctx.out(), "{}", value);
      } else if constexpr (std::is_same_v<T, bool>) {
        return std::format_to(ctx.out(), "{}", value ? "true" : "false");
      } else if constexpr (std::is_same_v<T, double>) {
        std::string text = std::format("{}", value);

        if (text.ends_with(".0")) {
          text = text.substr(0, text.length() - 2);
        }

        return std::format_to(ctx.out(), "{}", text);
      } else {
        return std::format_to(ctx.out(), "{}", value);
      }
    };

    return std::visit(visitor, obj);
  }
};

#endif
