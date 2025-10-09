#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <map>

#include "object.hpp"
#include "token.hpp"
class Environment {
 private:
  std::map<std::string, Object> m_values = {};
  Environment() = default;

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
