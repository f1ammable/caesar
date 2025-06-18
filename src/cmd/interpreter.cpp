#include "interpreter.hpp"

#include "ast.hpp"

Interpreter::Interpreter() : m_ans(0) {}

double Interpreter::answer() const { return m_ans; }

void Interpreter::visit(BinaryExpr* expr) {
  expr->children[0]->accept(this);
  double a = m_ans;
  expr->children[1]->accept(this);
  double b = m_ans;

  if (expr->token->value == "+")
    m_ans = a + b;
  else if (expr->token->value == "-")
    m_ans = a - b;
  else
    throw "Undefined operation";
}

void Interpreter::visit(NumLiteral* integer) {
  m_ans = atof(integer->token->value.c_str());
}
