#include "error.hpp"

#include <iostream>

#include "formatter.hpp"
#include "token.hpp"

void Error::_error(TokenType where, const std::string& msg, ErrorType type) {
  std::cerr << std::format("{} error at {} : {}", type, where, msg)
            << std::endl;
  hadError = true;
}

void Error::error(TokenType where, const std::string& msg, ErrorType type) {
  Error& e = Error::getInstance();
  e._error(where, msg, type);
}

Error::Error() : hadError(false) {}

Error& Error::getInstance() {
  static Error instance;
  return instance;
}
