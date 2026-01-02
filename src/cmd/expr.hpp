#ifndef EXPR_H
#define EXPR_H

#include <format>
#include <memory>
#include <utility>

#include "formatter.hpp"
#include "object.hpp"
#include "token.hpp"

class Binary;

class Grouping;

class Literal;

class Unary;

class Variable;

class Assign;

class IExprVisitor {
 public:
  virtual ~IExprVisitor() = default;

  virtual Object visitBinaryExpr(const Binary& expr) = 0;
  virtual Object visitGroupingExpr(const Grouping& expr) = 0;
  virtual Object visitLiteralExpr(const Literal& expr) = 0;
  virtual Object visitUnaryExpr(const Unary& expr) = 0;
  virtual Object visitVariableExpr(const Variable& expr) = 0;
  virtual Object visitAssignExpr(const Assign& expr) = 0;
};

class Expr {
 public:
  virtual ~Expr() = default;
  virtual Object accept(IExprVisitor* visitor) const = 0;
  [[nodiscard]] virtual constexpr std::string str() const = 0;
};

template <>
struct std::formatter<Expr> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const Expr& t, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "{}", t.str());
  }
};

template <>
struct std::formatter<Expr*> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(Expr* expr, auto& ctx) const {
    if (expr) {
      return std::format_to(ctx.out(), "{}", expr->str());
    }
    return std::format_to(ctx.out(), "null");
  }
};

template <>
struct std::formatter<std::unique_ptr<Expr>> : public std::formatter<Expr*> {
  auto format(const std::unique_ptr<Expr>& expr,
              std::format_context& ctx) const {
    return std::formatter<Expr*>::format(expr.get(), ctx);
  }
};

template <typename T>
  requires std::derived_from<T, Expr> && (!std::same_as<T, Expr>)
struct std::formatter<T> : std::formatter<Expr> {
  auto format(const T& expr, std::format_context& ctx) const {
    return std::formatter<Expr>::format(static_cast<const Expr&>(expr), ctx);
  }
};

class Binary final : public Expr {
 public:
  Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
      : m_left(std::move(left)),
        m_op(std::move(op)),
        m_right(std::move(right)) {}

  [[nodiscard]] Object accept(IExprVisitor* visitor) const override {
    return visitor->visitBinaryExpr(*this);
  }

  std::unique_ptr<Expr> m_left;
  Token m_op;
  std::unique_ptr<Expr> m_right;

  [[nodiscard]] constexpr std::string str() const override {
    return std::format("{} {} {}", m_op, m_left, m_right);
  }
};

class Grouping final : public Expr {
 public:
  explicit Grouping(std::unique_ptr<Expr> expr) : m_expr(std::move(expr)) {}

  Object accept(IExprVisitor* visitor) const override {
    return visitor->visitGroupingExpr(*this);
  }

  std::unique_ptr<Expr> m_expr;

  [[nodiscard]] constexpr std::string str() const override {
    return std::format("{}", m_expr);
  }
};

class Literal final : public Expr {
 public:
  explicit Literal(Object value) : m_value(std::move(value)) {}

  Object accept(IExprVisitor* visitor) const override {
    return visitor->visitLiteralExpr(*this);
  }

  Object m_value;

  [[nodiscard]] constexpr std::string str() const override {
    return std::format("{}", m_value);
  }
};

class Unary final : public Expr {
 public:
  Unary(Token op, std::unique_ptr<Expr> right)
      : m_op(std::move(op)), m_right(std::move(right)) {}

  Object accept(IExprVisitor* visitor) const override {
    return visitor->visitUnaryExpr(*this);
  }

  Token m_op;
  std::unique_ptr<Expr> m_right;

  [[nodiscard]] constexpr std::string str() const override {
    return std::format("{} {}", m_op, m_right);
  }
};

class Variable final : public Expr {
 public:
  Token m_name;

  explicit Variable(Token name) : m_name(name) {}

  Object accept(IExprVisitor* visitor) const override {
    return visitor->visitVariableExpr(*this);
  }

  [[nodiscard]] std::string str() const override {
    return std::format("{}", m_name);
  }
};

class Assign final : public Expr {
 public:
  Token m_name;
  std::unique_ptr<Expr> m_value;

  explicit Assign(Token name, std::unique_ptr<Expr> value)
      : m_name(name), m_value(std::move(value)) {}

  Object accept(IExprVisitor* visitor) const override {
    return visitor->visitAssignExpr(*this);
  }

  [[nodiscard]] std::string str() const override {
    return std::format("{} - {}", m_name, m_value);
  }
};

#endif
