#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <map>
#include <memory>

#include "object.hpp"
#include "stdlib.hpp"
#include "token.hpp"
class Environment {
 private:
  std::map<std::string, Object> m_values = {};
  inline Environment() {
    this->define("print", std::make_shared<PrintFn>(PrintFn()));
    this->define("len", std::make_shared<LenFn>(LenFn()));
  }

 public:
  void define(const std::string& name, Object value);
  Object get(Token name);
  void assign(Token name, Object value);

  Environment(Environment& other) = delete;
  Environment& operator=(const Environment& other) = delete;
  Environment(Environment&& other) = delete;
  Environment& operator=(Environment&& other) = delete;

  static Environment& getInstance();
};

#endif
