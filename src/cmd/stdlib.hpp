#ifndef STDLIB_H
#define STDLIB_H

#include <variant>

#include "callable.hpp"

class LenFn : public Callable {
 public:
  int arity() const override { return 1; }
  [[nodiscard]] std::string str() override { return "<native fn: len>"; }

  Object call(Interpreter& interpreter, std::vector<Object> args) override {
    if (args.size() != 1)
      throw RuntimeError("len() requires exactly one argument");

    auto* val = std::get_if<std::string>(&args[0]);

    if (!val) throw RuntimeError("len() can only be called on a string");

    return (int)val->length();
  }
};

#endif
