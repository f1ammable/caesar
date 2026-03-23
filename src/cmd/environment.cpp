#include "environment.hpp"

#include <format>
#include <utility>
#include <variant>

#include "core/context.hpp"
#include "core/platform.hpp"
#include "error.hpp"
#include "object.hpp"
#include "token.hpp"

void Environment::define(const std::string& name, Object value) {
  if (m_values.contains(name))
    if (auto* ptr = std::get_if<std::shared_ptr<Callable>>(&m_values[name])) {
      CmdError::error(
          TokenType::IDENTIFIER,
          std::format("Cannot assign to {} as it is a function", name),
          CmdErrorType::RUNTIME_ERROR);
      return;
    }

  m_values[name] = std::move(value);
}

Object Environment::get(const Token& name) {
  if (name.m_lexeme[0] == '$') return Environment::getRegister(name);

  if (m_values.contains(name.m_lexeme)) return m_values[name.m_lexeme];
  CmdError::error(name.m_type,
                  std::format("Undefined variable '{}'.", name.m_lexeme),
                  CmdErrorType::RUNTIME_ERROR);
  return std::monostate{};
}

void Environment::assign(const Token& name, Object value) {
  if (m_values.contains(name.m_lexeme)) {
    if (auto* val =
            std::get_if<std::shared_ptr<Callable>>(&m_values[name.m_lexeme]);
        val != nullptr) {
      CmdError::error(
          name.m_type,
          std::format("Cannot reassign {} as it is a function", name.m_lexeme),
          CmdErrorType::RUNTIME_ERROR);
      return;
    }
    m_values[name.m_lexeme] = std::move(value);
    return;
  }

  CmdError::error(name.m_type,
                  std::format("Undefined variable '{}'", name.m_lexeme),
                  CmdErrorType::RUNTIME_ERROR);
}

Environment& Environment::getInstance() {
  static Environment instance;
  return instance;
}

std::map<std::string, Object> Environment::getAll() { return m_values; }

Object Environment::getRegister(const Token& reg) {
  auto& target = Context::getTarget();

  std::string regName = reg.m_lexeme;
  regName.erase(0, 1);

  auto regEntry = findRegEntry(regName);
  if (!regEntry) {
    CmdError::error(reg.m_type, "$ prefix can only be used to access registers",
                    CmdErrorType::RUNTIME_ERROR);
    return std::monostate{};
  }

  return static_cast<double>(
      readRegValue(target->getLastKnownThreadState(), *regEntry.value()));
}
