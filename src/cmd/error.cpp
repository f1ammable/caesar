#include "error.hpp"

#include <iostream>

#include "formatter.hpp"

void Error::_error(TokenType where, const std::string& msg, ErrorType type) {
  std::cerr << std::format("{} error at {} : {}\n", type, where, msg);
  had_error = true;
}

void Error::error(TokenType where, const std::string& msg, ErrorType type) {
  Error& e = Error::getInstance();
  e._error(where, msg, type);
}

Error::Error() = default;

Error& Error::getInstance() {
  static Error instance;
  return instance;
}
