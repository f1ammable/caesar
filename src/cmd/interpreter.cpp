#include "interpreter.hpp"

#include <iostream>
#include <variant>

#include "error.hpp"
#include "expr.hpp"
#include "object.hpp"
#include "runtime_error.hpp"
#include "stmnt.hpp"
#include "token.hpp"

Object Interpreter::evaluate(const std::unique_ptr<Expr>& expr) {
  return expr->accept(this);
}

bool Interpreter::isTruthy(const Object& object) {
  auto visitor = [](const auto& value) -> bool {
    using T = std::decay_t<decltype(value)>;

    if constexpr (std::is_same_v<T, std::monostate>) return false;
    if constexpr (std::is_same_v<T, bool>)
      return value;
    else
      return true;
  };

  return std::visit(visitor, object);
}

bool Interpreter::isEqual(const Object& lhs, const Object& rhs) {
  return lhs == rhs;
}

void Interpreter::checkNumberOperand(const Token& op, const Object& operand) {
  auto visitor = [&op](const auto& value) -> void {
    using T = std::decay_t<decltype(value)>;

    if constexpr (std::is_same_v<T, double>) return;

    throw RuntimeError("Operand must be a number");
  };

  std::visit(visitor, operand);
}

std::string Interpreter::stringify(const Object& object) {
  return std::format("{}", object);
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmnt>>& stmnts) {
  try {
    for (const auto& s : stmnts) {
      execute(s);
    }
  } catch (RuntimeError& e) {
    Err& err = Err::getInstance();
    err.runtimeError(e);
  }
}

Object Interpreter::visitLiteralExpr(const Literal& expr) {
  return expr.m_value;
}

Object Interpreter::visitGroupingExpr(const Grouping& expr) {
  return evaluate(expr.m_expr);
}

Object Interpreter::visitUnaryExpr(const Unary& expr) {
  Object right = evaluate(expr.m_right);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
  switch (expr.m_op.m_type) {
    case TokenType::BANG:
      return !isTruthy(right);
    case TokenType::MINUS:
      checkNumberOperand(expr.m_op, right);
      return -(std::get<double>(right));
  }
#pragma clang diagnostic pop

  return std::monostate{};
}

Object Interpreter::visitBinaryExpr(const Binary& expr) {
  Object left = evaluate(expr.m_left);
  Object right = evaluate(expr.m_right);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
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
#pragma clang diagnostic pop

  return std::monostate{};
}

Object Interpreter::visitExprStmnt(const ExprStmnt& stmnt) {
  evaluate(stmnt.m_expr);
  return std::monostate{};
}

Object Interpreter::visitPrintStmnt(const PrintStmnt& stmnt) {
  Object value = evaluate(stmnt.m_expr);
  std::cout << stringify(value) << std::endl;
  return std::monostate{};
}

Object Interpreter::execute(const std::unique_ptr<Stmnt>& stmnt) {
  return stmnt->accept(this);
}

Object Interpreter::visitVarStmnt(const VarStmnt& stmnt) {
  Object value = std::monostate{};

  if (stmnt.m_initialiser != nullptr) {
    value = evaluate(stmnt.m_initialiser);
  }

  m_env.define(stmnt.m_name.m_lexeme, value);
  return std::monostate{};
}

Object Interpreter::visitVariableExpr(const Variable& expr) {
  return m_env.get(expr.m_name);
}

Object Interpreter::visitAssignExpr(const Assign& expr) {
  Object value = evaluate(expr.m_value);
  m_env.assign(expr.m_name, value);
  return value;
}
