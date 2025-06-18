#ifndef CAESAR_INTERPRETER_HPP
#define CAESAR_INTERPRETER_HPP

#include "visitor.hpp"

class Interpreter : public Visitor {
 private:
  double m_ans;

 public:
  explicit Interpreter();
  CONCRETE_VISIT_METHOD_DECL(BinaryExpr);
  CONCRETE_VISIT_METHOD_DECL(NumLiteral);

  double answer() const;
};

#endif