#ifndef ERROR_H
#define ERROR_H
#include "runtime_error.hpp"
#include "token.hpp"
#include <string>

class Err {
private:
  Err();

public:
  Err(Err &other) = delete;
  Err &operator=(const Err &other) = delete;
  Err(Err &&other) = delete;
  Err &operator=(Err &&other) = delete;

  bool hadError;
  bool hadRuntimeError;
  void report(int line, const std::string &where, const std::string &msg);
  void error(int line, const std::string &msg);
  void error(Token token, const std::string &msg);
  void runtimeError(RuntimeError &err);
  static Err &getInstance();
};

#endif
