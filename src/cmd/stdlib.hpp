#ifndef STDLIB_H
#define STDLIB_H

#include <cstdint>
#include <variant>

#include "callable.hpp"
#include "error.hpp"
#include "formatter.hpp"
#include "subcommand.hpp"
#include "token_type.hpp"

class LenFn : public Callable {
 public:
  [[nodiscard]] int arity() const override { return 1; }
  [[nodiscard]] std::string str() const override { return "<native fn: len>"; }

  Object call(std::vector<Object> args) override {
    auto* val = std::get_if<std::string>(args.data());

    if (val == nullptr) {
      Error::error(TokenType::IDENTIFIER, "len can only be called on a string",
                   ErrorType::RUNTIME_ERROR);
      return std::monostate{};
    }

    return static_cast<double>(val->length());
  }
};

class PrintFn : public Callable {
 public:
  [[nodiscard]] int arity() const override { return 1; }
  [[nodiscard]] std::string str() const override {
    return "<native fn: print>";
  }

  Object call(std::vector<Object> args) override {
    return std::format("{}", args[0]);
  }
};

class BreakpointFn : public Callable {
 private:
  using sv = std::string_view;
  using FnPtr = Object (*)(const std::vector<std::string>&);

  FnPtr l = [](const std::vector<std::string>& x) {
    return Object{"test from subcommand!"};
  };

  SubcommandHandler m_subcmds{{{sv("list"), l}}, "breakpoint"};

  static std::vector<std::string> convertToStr(const std::vector<Object>& vec) {
    std::vector<std::string> res{};
    res.reserve(vec.size());

    for (const auto& x : vec) {
      const auto* const str = std::get_if<std::string>(&x);
      if (str == nullptr) {
        Error::error(TokenType::IDENTIFIER,
                     "Please provide all arguments as strings",
                     ErrorType::RUNTIME_ERROR);
        break;
      }
      res.emplace_back(*str);
    }

    return res;
  }

 public:
  [[nodiscard]] int arity() const override { return 1; }
  [[nodiscard]] std::string str() const override {
    return "<native fn: breakpoint>";
  }

  Object call(std::vector<Object> args) override {
    std::vector<std::string> convertedArgs = BreakpointFn::convertToStr(args);
    if (convertedArgs.size() < 1) return std::monostate{};
    const std::string subcmd = convertedArgs.front();
    convertedArgs.erase(convertedArgs.begin());
    return m_subcmds.exec(subcmd, convertedArgs);
  }
};

#endif
