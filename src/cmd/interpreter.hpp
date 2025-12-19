#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "environment.hpp"
#include "expr.hpp"
#include "object.hpp"
#include "stmnt.hpp"

class Interpreter : public IExprVisitor, IStmntVisitor {
 private:
  Environment& m_env = Environment::getInstance();

  Object evaluate(const std::unique_ptr<Expr>& expr);
  bool isTruthy(const Object& object);
  bool isEqual(const Object& lhs, const Object& rhs);
  void checkNumberOperand(const Token& op, const Object& operand);
  std::string stringify(const Object& object);
  Object execute(const std::unique_ptr<Stmnt>& stmnt);

 public:
  Object visitLiteralExpr(const Literal& expr) override;
  Object visitGroupingExpr(const Grouping& expr) override;
  Object visitUnaryExpr(const Unary& expr) override;
  Object visitBinaryExpr(const Binary& expr) override;
  Object visitVariableExpr(const Variable& expr) override;
  void interpret(const std::vector<std::unique_ptr<Stmnt>>& stmnts);
  Object visitExprStmnt(const ExprStmnt& stmnt) override;
  Object visitPrintStmnt(const PrintStmnt& stmnt) override;
  Object visitVarStmnt(const VarStmnt& stmnt) override;
  Object visitAssignExpr(const Assign& expr) override;
  Object visitCallStmnt(const CallStmnt& stmnt) override;
};

#endif  // !INTERPRETER_HPP
