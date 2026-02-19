#include "interpreter.hpp"

#include <format>
#include <variant>

#include "callable.hpp"
#include "error.hpp"
#include "expr.hpp"
#include "object.hpp"
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

bool Interpreter::checkNumberOperand(const Token& op, const Object& operand) {
  auto visitor = [&op](const auto& value) -> bool {
    using T = std::decay_t<decltype(value)>;

    if constexpr (std::is_same_v<T, double>) return true;

    CmdError::error(op.m_type, "Operand must be a number",
                    CmdErrorType::RUNTIME_ERROR);
    return false;
  };

  return std::visit(visitor, operand);
}

std::string Interpreter::stringify(const Object& object) {
  return std::format("{}", object);
}

Object Interpreter::interpret(const std::unique_ptr<Stmnt>& stmnt) {
  return execute(stmnt);
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
      if (!checkNumberOperand(expr.m_op, right)) return std::monostate{};
      return -(std::get<double>(right));
  }
#pragma clang diagnostic pop

  return std::monostate{};
}

[[nodiscard]] Object Interpreter::visitBinaryExpr(const Binary& expr) {
  const Object left = evaluate(expr.m_left);
  const Object right = evaluate(expr.m_right);

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
  return evaluate(stmnt.m_expr);
}

Object Interpreter::visitCallStmnt(const CallStmnt& stmnt) {
  // TODO: Use array instead?
  std::vector<Object> argList = {};
  argList.reserve(stmnt.m_args.size());

  for (const auto& x : stmnt.m_args) {
    if (auto* id = dynamic_cast<Variable*>(x.get());
        id != nullptr && !(m_env.getAll().contains(id->m_name.m_lexeme))) {
      argList.emplace_back(id->m_name.m_lexeme);
      continue;
    }

    argList.emplace_back(std::move(evaluate(x)));
  }

  Object fnObj = m_env.get(stmnt.m_fn);
  auto* fn = std::get_if<std::shared_ptr<Callable>>(&fnObj);

  if (fn == nullptr) {
    // Not a callable - if no args, just return the value
    if (argList.empty()) return fnObj;
    CmdError::error(stmnt.m_fn.m_type,
                    std::format("'{}' is not callable", stmnt.m_fn.m_lexeme),
                    CmdErrorType::RUNTIME_ERROR);
    return std::monostate{};
  }

  if (argList.size() < fn->get()->arity()) {
    CmdError::error(stmnt.m_fn.m_type,
                    std::format("Function requires at least {} arguments but "
                                "only {} were provided",
                                fn->get()->arity(), argList.size()),
                    CmdErrorType::RUNTIME_ERROR);
    return std::monostate{};
  }

  return fn->get()->call(std::move(argList));
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
