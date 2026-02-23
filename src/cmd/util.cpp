#include "util.hpp"

#include <format>
#include <iostream>

bool detail::isop(const char c) {
  return c == '+' || c == '-' || c == '*' || c == '/';
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
    size_t pos = 0;  // NOLINT(misc-const-correctness)
    const int val = std::stoi(str, &pos);
    if (pos == str.length() && str.find('.') == std::string::npos) {
      return val;
    }
  } catch (...) {
    std::cerr << std::format("Error parsing number: '{}'\n", str);
  }

  try {
    const float fVal = std::stof(str);
    const double dVal = std::stod(str);

    if (std::abs(fVal - dVal) < 1e-16) {
      return fVal;
    }
    return dVal;
  } catch (...) {
    throw std::invalid_argument(
        std::format("Cannot parse {} as number\n", str));
  }
}

std::vector<std::string> detail::convertToStr(const std::vector<Object>& vec) {
  std::vector<std::string> res{};
  res.reserve(vec.size());

  for (const auto& x : vec) {
    const auto* const str = std::get_if<std::string>(&x);
    if (str == nullptr) {
      CmdError::error(TokenType::IDENTIFIER,
                      "Please provide all arguments as strings",
                      CmdErrorType::RUNTIME_ERROR);
      return {};  // Return empty on error, not partial
    }
    res.emplace_back(*str);
  }

  return res;
}
