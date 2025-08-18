#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "expr.hpp"

class Interpreter : public IExprVisitor {
private:
  Object evaluate(const std::unique_ptr<Expr> &expr);
  bool isTruthy(const Object &object);
  bool isEqual(const Object &lhs, const Object &rhs);
  void checkNumberOperand(const Token &op, const Object &operand);
  std::string stringify(const Object &object);

public:
  Object visitLiteralExpr(const Literal &expr) override;
  Object visitGroupingExpr(const Grouping &expr) override;
  Object visitUnaryExpr(const Unary &expr) override;
  Object visitBinaryExpr(const Binary &expr) override;
  void interpret(const std::unique_ptr<Expr> &expression);
};

#endif // !INTERPRETER_HPP
