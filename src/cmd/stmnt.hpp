#ifndef STMNT_HPP
#define STMNT_HPP

#include "expr.hpp"
#include "object.hpp"
#include <memory>

class ExprStmnt;
class PrintStmnt;

class IStmntVisitor {
public:
  virtual ~IStmntVisitor() = default;

  virtual Object visitExprStmnt(const ExprStmnt &stmnt) = 0;
  virtual Object visitPrintStmnt(const PrintStmnt &stmnt) = 0;
};

class Stmnt {
public:
  virtual ~Stmnt() = default;
  virtual Object accept(IStmntVisitor *visitor) = 0;
};

class ExprStmnt final : public Stmnt {
public:
  std::unique_ptr<Expr> m_expr;

  ExprStmnt(std::unique_ptr<Expr> expr) : m_expr(std::move(expr)) {}

  Object accept(IStmntVisitor *visitor) override {
    return visitor->visitExprStmnt(*this);
  }
};

class PrintStmnt final : public Stmnt {
public:
  std::unique_ptr<Expr> m_expr;

  PrintStmnt(std::unique_ptr<Expr> expr) : m_expr(std::move(expr)) {}

  Object accept(IStmntVisitor *visitor) override {
    return visitor->visitPrintStmnt(*this);
  }
};

#endif // !STMNT_HPP
