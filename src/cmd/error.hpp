#ifndef ERROR_H
#define ERROR_H
#include <cstdint>
#include <format>
#include <map>
#include <string>

enum class ErrorType : std::uint8_t { PARSE_ERROR, SCAN_ERROR, RUNTIME_ERROR };
enum class TokenType : std::uint8_t;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Error {
 private:
  Error();
  // NOLINTNEXTLINE(readability-identifier-naming)
  void _error(TokenType where, const std::string& msg, ErrorType type);

 public:
  Error(Error& other) = delete;
  Error& operator=(const Error& other) = delete;
  Error(Error&& other) = delete;
  Error& operator=(Error&& other) = delete;

  bool had_error{false};
  static void error(TokenType where, const std::string& msg, ErrorType type);
  static Error& getInstance();
};

template <>
struct std::formatter<ErrorType> : std::formatter<std::string_view> {
  constexpr auto format(ErrorType type, auto& ctx) const {
    const auto str = [] {
      std::map<ErrorType, std::string> res;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INSERT_ELEM(p) res.emplace(p, #p);
      INSERT_ELEM(ErrorType::PARSE_ERROR);
      INSERT_ELEM(ErrorType::SCAN_ERROR);
      INSERT_ELEM(ErrorType::RUNTIME_ERROR);
#undef INSERT_ELEM

      return res;
    };

    return std::formatter<std::string_view>::format(str()[type], ctx);
  };
};

#endif
