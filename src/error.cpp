#include "error.hpp"

#include <iostream>

#include "formatter.hpp"

void CoreError::_error(const std::string& msg) {
  std::cerr << std::format("Error : {}\n", msg);
  m_had_error = true;
}

void CoreError::error(const std::string& msg) {
  CoreError& e = CoreError::getInstance();
  e._error(msg);
}

CoreError& CoreError::getInstance() {
  static CoreError instance;
  return instance;
}

void CmdError::_error(TokenType where, const std::string& msg,
                      CmdErrorType type) {
  std::cerr << std::format("{} error at {} : {}\n", type, where, msg);
  m_had_error = true;
}

void CmdError::error(TokenType where, const std::string& msg,
                     CmdErrorType type) {
  CmdError& e = CmdError::getInstance();
  e._error(where, msg, type);
}

CmdError& CmdError::getInstance() {
  static CmdError instance;
  return instance;
}
