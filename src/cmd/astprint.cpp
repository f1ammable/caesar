#include "astprint.hpp"

Object AstPrinter::print(const std::unique_ptr<Expr>& expr) {
  return expr->accept(this);
}

Object AstPrinter::visitBinaryExpr(const Binary& expr) {
  return parenthesise(expr.m_op.m_lexeme, expr.m_left, expr.m_right);
}

Object AstPrinter::visitUnaryExpr(const Unary& expr) {
  return parenthesise(expr.m_op.m_lexeme, expr.m_right);
}

Object AstPrinter::visitGroupingExpr(const Grouping& expr) {
  return parenthesise("group", expr.m_expr);
}

Object AstPrinter::visitLiteralExpr(const Literal& expr) {
  return std::format("{}", expr);
}
