#ifndef CALLABLE_H
#define CALLABLE_H

#include "interpreter.hpp"

class Callable {
 public:
  virtual ~Callable() = default;
  virtual Object call(Interpreter& interpreter, std::vector<Object> args) = 0;
  virtual int arity() = 0;
};

#endif
