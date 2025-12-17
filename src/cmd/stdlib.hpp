#ifndef STDLIB_H
#define STDLIB_H

#include <variant>

#include "formatter.hpp"

class LenFn : public Callable {
 public:
  int arity() const override { return 1; }
  [[nodiscard]] std::string str() const override { return "<native fn: len>"; }

  Object call(Interpreter& interpreter, std::vector<Object> args) override {
    auto* val = std::get_if<std::string>(&args[0]);

    if (!val) throw RuntimeError("len can only be called on a string");

    return (double)val->length();
  }
};

class PrintFn : public Callable {
 public:
  int arity() const override { return 1; }
  [[nodiscard]] std::string str() const override {
    return "<native fn: print>";
  }

  Object call(Interpreter& interpreter, std::vector<Object> args) override {
    return std::format("{}", args[0]);
  }
};

#endif
