#ifndef RUNTIME_ERROR_HPP
#define RUNTIME_ERROR_HPP

#include <exception>
#include <string>

class RuntimeError : public std::exception {
private:
  std::string message;

public:
  inline RuntimeError() : message("Runtime error occured.") {}
  inline RuntimeError(const std::string &msg) : message(msg) {}
  inline const char *what() { return message.c_str(); }
};

#endif // !RUNTIME_ERROR_HPP
