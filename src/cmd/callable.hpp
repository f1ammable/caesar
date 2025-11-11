#ifndef CALLABLE_H
#define CALLABLE_H

#include <string>
#include <vector>

#include "object.hpp"

class Interpreter;

class Callable {
 public:
  virtual ~Callable() = default;
  virtual Object call(Interpreter& interpreter, std::vector<Object> args) = 0;
  virtual int arity() const = 0;
  [[nodiscard]] virtual std::string str() = 0;
};

#endif
