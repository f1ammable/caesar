#include "environment.hpp"

#include <format>

#include "object.hpp"
#include "runtime_error.hpp"
#include "token.hpp"

void Environment::define(const std::string& name, Object value) {
  m_values[name] = value;
}

Object Environment::get(Token name) {
  if (m_values.contains(name.m_lexeme)) return m_values[name.m_lexeme];
  throw RuntimeError(std::format("Undefined variable '{}'.", name.m_lexeme));
}

void Environment::assign(Token name, Object value) {
  if (m_values.contains(name.m_lexeme)) {
    m_values[name.m_lexeme] = value;
  }

  throw RuntimeError(std::format("Undefined variable '{}'", name.m_lexeme));
}

Environment& Environment::getInstance() {
  static Environment instance;
  return instance;
}
