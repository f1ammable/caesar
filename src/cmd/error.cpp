#include "error.hpp"

#include <iostream>

#include "runtime_error.hpp"

void Err::report(const std::string& where, const std::string& msg) {
  std::cout << std::format("Error {}: {}\n", where, msg);
  hadError = true;
}

void Err::error(const std::string& msg) {
  report("", msg);
}

void Err::error(Token token, const std::string& msg) {
  if (token.m_type == TokenType::END) {
    report(" at end", msg);
  } else {
    report(std::format("at '{}'", token.m_lexeme), msg);
  }
}

void Err::runtimeError(RuntimeError& err) {
  std::cerr << err.what() << std::endl;
  hadRuntimeError = true;
}

Err::Err() : hadError(false), hadRuntimeError(false) {}

Err& Err::getInstance() {
  static Err instance;
  return instance;
}
