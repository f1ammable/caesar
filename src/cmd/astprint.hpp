#ifndef ASTPRINT_H
#define ASTPRINT_H
#include "expr.hpp"

class AstPrinter : public IExprVisitor {
 private:
  template <typename... Args>
    requires(std::same_as<std::decay_t<Args>, std::unique_ptr<Expr>> && ...)
  std::string parenthesise(const std::string& name, Args&... expr) {
    std::string result;
    result.append("(").append(name);
    (result.append(std::format(" {}", expr->accept(this))), ...);
    result.append(")");
    return result;
  }

 public:
  Object print(const std::unique_ptr<Expr>& expr);
  Object visitBinaryExpr(const Binary& expr) override;
  Object visitUnaryExpr(const Unary& expr) override;
  Object visitGroupingExpr(const Grouping& expr) override;
  Object visitLiteralExpr(const Literal& expr) override;
};

#endif
