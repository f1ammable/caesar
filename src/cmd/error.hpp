#ifndef ERROR_H
#define ERROR_H
#include <format>
#include <map>
#include <string>

enum class ErrorType { ParseError, ScanError, RuntimeError };
enum class TokenType;

class Error {
 private:
  Error();
  void _error(TokenType where, const std::string& msg, ErrorType type);

 public:
  Error(Error& other) = delete;
  Error& operator=(const Error& other) = delete;
  Error(Error&& other) = delete;
  Error& operator=(Error&& other) = delete;

  bool hadError;
  static void error(TokenType where, const std::string& msg, ErrorType type);
  static Error& getInstance();
};

template <>
struct std::formatter<ErrorType> : std::formatter<std::string_view> {
  constexpr auto format(ErrorType type, auto& ctx) const {
    const auto str = [] {
      std::map<ErrorType, std::string> res;
#define INSERT_ELEM(p) res.emplace(p, #p);
      INSERT_ELEM(ErrorType::ParseError);
      INSERT_ELEM(ErrorType::ScanError);
      INSERT_ELEM(ErrorType::RuntimeError);
#undef INSERT_ELEM

      return res;
    };

    return std::formatter<std::string_view>::format(str()[type], ctx);
  };
};

#endif
