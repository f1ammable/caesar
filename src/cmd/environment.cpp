#include "environment.hpp"

#include <format>
#include <variant>

#include "error.hpp"
#include "object.hpp"
#include "token.hpp"

void Environment::define(const std::string& name, Object value) {
  if (m_values.contains(name))
    if (auto ptr = std::get_if<std::shared_ptr<Callable>>(&m_values[name])) {
      Error::error(TokenType::IDENTIFIER,
                   std::format("Cannot assign to {} as it is a function", name),
                   ErrorType::RuntimeError);
      return;
    }

  m_values[name] = value;
}

Object Environment::get(Token name) {
  if (m_values.contains(name.m_lexeme)) return m_values[name.m_lexeme];
  Error::error(name.m_type,
               std::format("Undefined variable '{}'.", name.m_lexeme),
               ErrorType::RuntimeError);
  return std::monostate{};
}

void Environment::assign(Token name, Object value) {
  if (m_values.contains(name.m_lexeme)) {
    if (auto* val =
            std::get_if<std::shared_ptr<Callable>>(&m_values[name.m_lexeme]);
        val != nullptr) {
      Error::error(
          name.m_type,
          std::format("Cannot reassign {} as it is a function", name.m_lexeme),
          ErrorType::RuntimeError);
      return;
    }
    m_values[name.m_lexeme] = value;
    return;
  }

  Error::error(name.m_type,
               std::format("Undefined variable '{}'", name.m_lexeme),
               ErrorType::RuntimeError);
}

Environment& Environment::getInstance() {
  static Environment instance;
  return instance;
}
