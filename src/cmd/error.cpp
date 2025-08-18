#include "error.hpp"
#include "runtime_error.hpp"

#include <iostream>

void Err::report(int line, const std::string &where, const std::string &msg) {
  std::cout << std::format("[line {}] Error {}: {}\n", line, where, msg);
  hadError = true;
}

void Err::error(const int line, const std::string &msg) {
  report(line, "", msg);
}

void Err::error(Token token, const std::string &msg) {
  if (token.m_type == TokenType::END) {
    report(token.m_line, " at end", msg);
  } else {
    report(token.m_line, std::format("at '{}'", token.m_lexeme), msg);
  }
}

void Err::runtimeError(RuntimeError &err) {
  std::cerr << std::format("{}\n[line ????]", err.what()) << std::endl;
  hadRuntimeError = true;
}

Err::Err() : hadError(false), hadRuntimeError(false) {}

Err &Err::getInstance() {
  static Err instance;
  return instance;
}
