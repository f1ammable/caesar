#include "interpreter.hpp"
#include "error.hpp"
#include "expr.hpp"
#include "runtime_error.hpp"
#include "token.hpp"
#include <iostream>
#include <variant>

Object Interpreter::evaluate(const std::unique_ptr<Expr> &expr) {
  return expr->accept(this);
}

bool Interpreter::isTruthy(const Object &object) {
  auto visitor = [](const auto &value) -> bool {
    using T = std::decay_t<decltype(value)>;

    if constexpr (std::is_same_v<T, std::monostate>)
      return false;
    if constexpr (std::is_same_v<T, bool>)
      return value;
    else
      return true;
  };

  return std::visit(visitor, object);
}

bool Interpreter::isEqual(const Object &lhs, const Object &rhs) {
  return lhs == rhs;
}

void Interpreter::checkNumberOperand(const Token &op, const Object &operand) {
  auto visitor = [&op](const auto &value) -> void {
    using T = std::decay_t<decltype(value)>;

    if constexpr (std::is_same_v<T, double>)
      return;

    throw RuntimeError("Operand must be a number");
  };

  std::visit(visitor, operand);
}

std::string Interpreter::stringify(const Object &object) {
  return std::format("{}", object);
}

void Interpreter::interpret(const std::unique_ptr<Expr> &expression) {
  try {
    Object val = evaluate(expression);
    std::cout << stringify(val) << std::endl;
  } catch (RuntimeError &err) {
    Err &e = Err::getInstance();
    e.runtimeError(err);
  }
}

Object Interpreter::visitLiteralExpr(const Literal &expr) {
  return expr.m_value;
}

Object Interpreter::visitGroupingExpr(const Grouping &expr) {
  return evaluate(expr.m_expr);
}

Object Interpreter::visitUnaryExpr(const Unary &expr) {
  Object right = evaluate(expr.m_right);

  switch (expr.m_op.m_type) {
  case TokenType::BANG:
    return !isTruthy(right);
  case TokenType::MINUS:
    checkNumberOperand(expr.m_op, right);
    return -(std::get<double>(right));
  }

  return std::monostate{};
}

Object Interpreter::visitBinaryExpr(const Binary &expr) {
  Object left = evaluate(expr.m_left);
  Object right = evaluate(expr.m_right);

  switch (expr.m_op.m_type) {
  case TokenType::GREATER:
    return left > right;
  case TokenType::GREATER_EQUAL:
    return left >= right;
  case TokenType::LESS:
    return left < right;
  case TokenType::LESS_EQUAL:
    return left <= right;
  case TokenType::BANG_EQUAL:
    return !isEqual(left, right);
  case TokenType::EQUAL_EQUAL:
    return isEqual(left, right);
  case TokenType::MINUS:
    return left - right;
  case TokenType::PLUS:
    return left + right;
  case TokenType::SLASH:
    return left / right;
  case TokenType::STAR:
    return left * right;
  }

  return std::monostate{};
}
