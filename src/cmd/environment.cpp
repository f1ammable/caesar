#include "environment.hpp"

#include <format>
#include <variant>

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
      if (auto* val = std::get_if<std::shared_ptr<Callable>>(&m_values[name.m_lexeme]); val != nullptr)
      throw RuntimeError(std::format("Cannot reassign {} as it is a function", name.m_lexeme));
    m_values[name.m_lexeme] = value;
    return;
  }

  throw RuntimeError(std::format("Undefined variable '{}'", name.m_lexeme));
}

Environment& Environment::getInstance() {
  static Environment instance;
  return instance;
}
