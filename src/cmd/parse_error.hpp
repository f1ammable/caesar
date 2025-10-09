#ifndef PARSE_ERR_HPP
#define PARSE_ERR_HPP

#include <exception>
#include <string>

class ParseError : public std::exception {
 private:
  std::string message;

 public:
  inline ParseError() : message("Parsing error occured.") {}
  inline ParseError(char* msg) : message(msg) {}
  inline const char* what() { return message.c_str(); }
};

#endif  // !PARSE_ERR_HPP
