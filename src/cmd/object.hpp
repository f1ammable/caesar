#ifndef OBJECT_H
#define OBJECT_H

#include <cstring>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

#include "error.hpp"
#include "token_type.hpp"

class Callable;

using Object = std::variant<std::monostate, std::string, double, bool,
                            std::shared_ptr<Callable>>;

template <typename Op>
inline Object binaryOperation(const Object& lhs, const Object& rhs, Op op,
                              const std::string& opName) {
  auto visitor = [&op, &opName](const auto& left, const auto& right) -> Object {
    using L = std::decay_t<decltype(left)>;
    using R = std::decay_t<decltype(right)>;

    if constexpr (std::is_arithmetic_v<L> && std::is_arithmetic_v<R>) {
      return op(static_cast<double>(left), static_cast<double>(right));
    } else if constexpr (std::is_same_v<L, std::string> &&
                         std::is_same_v<R, std::string>) {
      if constexpr (std::is_same_v<Op, std::plus<>>) {
        return left + right;
      } else {
        Error::error(TokenType::STRING,
                     std::format("Cannot apply operator {} to strings", opName),
                     ErrorType::RUNTIME_ERROR);
        return std::monostate{};
      }
    } else {
      Error::error(TokenType::STRING,
                   std::format("Unsupported operand types for {}", opName),
                   ErrorType::RUNTIME_ERROR);
      return std::monostate{};
    }
  };

  return std::visit(visitor, lhs, rhs);
}

template <typename Op>
inline bool comparisonOperation(const Object& lhs, const Object& rhs, Op op,
                                const std::string& opName) {
  auto visitor = [&op, &opName](const auto& left, const auto& right) -> bool {
    using L = std::decay_t<decltype(left)>;
    using R = std::decay_t<decltype(right)>;

    if constexpr (std::is_arithmetic_v<L> && std::is_arithmetic_v<R>) {
      return op(static_cast<double>(left), static_cast<double>(right));
    } else {
      Error::error(
          TokenType::STRING,
          std::format("Cannot apply operator {} to non-arithmetic types",
                      opName),
          ErrorType::RUNTIME_ERROR);
      return false;
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

    return binaryOperation(left, right, std::plus<double>{}, "+");
  };

  return std::visit(visitor, lhs, rhs);
}

inline Object operator-(const Object& lhs, const Object& rhs) {
  return binaryOperation(lhs, rhs, std::minus<double>{}, "-");
}

inline Object operator/(const Object& lhs, const Object& rhs) {
  return binaryOperation(lhs, rhs, std::divides<double>{}, "/");
}

inline Object operator*(const Object& lhs, const Object& rhs) {
  return binaryOperation(lhs, rhs, std::multiplies<double>{}, "*");
}

inline bool operator==(const Object& lhs, const Object& rhs) {
  if (lhs.index() == rhs.index()) {
    return std::visit([](const auto& l, const auto& r) { return l == r; }, lhs,
                      rhs);
  };

  return false;
}

inline bool operator!=(const Object& lhs, const Object& rhs) {
  return !(lhs == rhs);
}

inline bool operator<(const Object& lhs, const Object& rhs) {
  return comparisonOperation(lhs, rhs, std::less<double>{}, "<");
}

inline bool operator>(const Object& lhs, const Object& rhs) {
  return comparisonOperation(lhs, rhs, std::greater<double>{}, ">");
}

inline bool operator<=(const Object& lhs, const Object& rhs) {
  return comparisonOperation(lhs, rhs, std::less_equal<double>{}, "<=");
}

inline bool operator>=(const Object& lhs, const Object& rhs) {
  return comparisonOperation(lhs, rhs, std::greater_equal<double>{}, ">=");
}

namespace detail {
template <typename T>
  requires(std::is_arithmetic_v<T>)
[[nodiscard]] std::string toString(const T& val, const int p = 3) {
  std::ostringstream out;
  out.precision(p);
  out << std::fixed << val;
  return std::move(out).str();
}

inline std::string boolToS(bool b) { return b ? "(true)" : "(false)"; }
}  // namespace detail

#endif
