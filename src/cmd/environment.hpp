#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <map>
#include <memory>

#include "object.hpp"
#include "stdlib.hpp"
#include "token.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Environment {
 private:
  std::map<std::string, Object> m_values;
  Environment() : m_values({}) {
    this->define("print", std::make_shared<PrintFn>(PrintFn()));
    this->define("len", std::make_shared<LenFn>(LenFn()));
    this->define("breakpoint", std::make_shared<BreakpointFn>(BreakpointFn()));
    this->define("run", std::make_shared<RunFn>(RunFn()));
    this->define("resume", std::make_shared<ContinueFn>(ContinueFn()));
    this->define("target", std::make_shared<TargetFn>(TargetFn()));
  }

 public:
  void define(const std::string& name, Object value);
  Object get(const Token& name);
  std::map<std::string, Object> getAll();
  void assign(const Token& name, Object value);

  Environment(Environment& other) = delete;
  Environment& operator=(const Environment& other) = delete;
  Environment(Environment&& other) = delete;
  Environment& operator=(Environment&& other) = delete;

  static Environment& getInstance();
};

#endif
