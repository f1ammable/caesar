#include "util.hpp"

#include <iostream>

bool detail::isop(const char c) {
  if (c == '+' || c == '-' || c == '*' || c == '/') return true;
  return false;
}

TokenType detail::ctoop(const char c) {
  switch (c) {
    case '+':
      return TokenType::PLUS;
    case '-':
      return TokenType::MINUS;
    case '*':
      return TokenType::STAR;
    case '/':
      return TokenType::SLASH;
    default:
      return TokenType::END;
  }
}

double detail::parseNumber(const std::string& str) {
  try {
    size_t pos = 0;
    int val = std::stoi(str, &pos);
    if (pos == str.length() && str.find('.') == std::string::npos) {
      return val;
    }
  } catch (...) {
    std::cerr << std::format("Error parsing number: '{}'\n", str);
  }

  try {
    float f_val = std::stof(str);
    double d_val = std::stod(str);

    if (std::abs(f_val - d_val) < 1e-16) {
      return f_val;
    }
    return d_val;
  } catch (...) {
    throw std::invalid_argument(
        std::format("Cannot parse {} as number\n", str));
  }
}
