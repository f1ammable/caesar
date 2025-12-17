#ifndef STMNT_HPP
#define STMNT_HPP

#include <memory>

#include "expr.hpp"
#include "object.hpp"
#include "token.hpp"

class ExprStmnt;
class PrintStmnt;
class VarStmnt;
class CallStmnt;

class IStmntVisitor {
 public:
  virtual ~IStmntVisitor() = default;

  virtual Object visitExprStmnt(const ExprStmnt& stmnt) = 0;
  virtual Object visitPrintStmnt(const PrintStmnt& stmnt) = 0;
  virtual Object visitVarStmnt(const VarStmnt& stmnt) = 0;
  virtual Object visitCallStmnt(const CallStmnt& stmnt) = 0;
};

class Stmnt {
 public:
  virtual ~Stmnt() = default;
  virtual Object accept(IStmntVisitor* visitor) = 0;
};

class ExprStmnt final : public Stmnt {
 public:
  std::unique_ptr<Expr> m_expr;

  ExprStmnt(std::unique_ptr<Expr> expr) : m_expr(std::move(expr)) {}

  Object accept(IStmntVisitor* visitor) override {
    return visitor->visitExprStmnt(*this);
  }
};

class PrintStmnt final : public Stmnt {
 public:
  std::unique_ptr<Expr> m_expr;

  PrintStmnt(std::unique_ptr<Expr> expr) : m_expr(std::move(expr)) {}

  Object accept(IStmntVisitor* visitor) override {
    return visitor->visitPrintStmnt(*this);
  }
};

class CallStmnt final : public Stmnt {
 public:
  Token m_fn;
  std::vector<std::unique_ptr<Expr>> m_args;

  CallStmnt(Token fn, std::vector<std::unique_ptr<Expr>> expr)
      : m_fn(std::move(fn)), m_args(std::move(expr)) {}

  Object accept(IStmntVisitor* visitor) override {
    return visitor->visitCallStmnt(*this);
  }
};

class VarStmnt final : public Stmnt {
 public:
  std::unique_ptr<Expr> m_initialiser;
  Token m_name;

  VarStmnt(Token name, std::unique_ptr<Expr> expr)
      : m_initialiser(std::move(expr)), m_name(name) {}

  Object accept(IStmntVisitor* visitor) override {
    return visitor->visitVarStmnt(*this);
  }
};

#endif  // !STMNT_HPP
