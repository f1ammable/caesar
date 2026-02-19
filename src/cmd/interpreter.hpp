#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "environment.hpp"
#include "error.hpp"
#include "expr.hpp"
#include "object.hpp"
#include "stmnt.hpp"

class Interpreter : public IExprVisitor, IStmntVisitor {
 private:
  Environment& m_env = Environment::getInstance();
  CmdError& err = CmdError::getInstance();

  Object evaluate(const std::unique_ptr<Expr>& expr);
  static bool isTruthy(const Object& object);
  static bool isEqual(const Object& lhs, const Object& rhs);
  static bool checkNumberOperand(const Token& op, const Object& operand);
  Object execute(const std::unique_ptr<Stmnt>& stmnt);

 public:
  Object visitLiteralExpr(const Literal& expr) override;
  Object visitGroupingExpr(const Grouping& expr) override;
  Object visitUnaryExpr(const Unary& expr) override;
  Object visitBinaryExpr(const Binary& expr) override;
  Object visitVariableExpr(const Variable& expr) override;
  Object interpret(const std::unique_ptr<Stmnt>& stmnt);
  Object visitExprStmnt(const ExprStmnt& stmnt) override;
  Object visitVarStmnt(const VarStmnt& stmnt) override;
  Object visitAssignExpr(const Assign& expr) override;
  Object visitCallStmnt(const CallStmnt& stmnt) override;
  static std::string stringify(const Object& object);
};

#endif  // !INTERPRETER_HPP
