#ifndef CMD_ERROR_H
#define CMD_ERROR_H

#include <cstdint>
#include <format>
#include <map>
#include <string>

enum class CmdErrorType : std::uint8_t {
  PARSE_ERROR,
  SCAN_ERROR,
  RUNTIME_ERROR
};
enum class TokenType : std::uint8_t;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class CoreError {
 private:
  CoreError() = default;
  // NOLINTNEXTLINE(readability-identifier-naming)
  void _error(const std::string& msg);

 public:
  CoreError(const CoreError&) = delete;
  CoreError(CoreError&&) = delete;
  CoreError& operator=(const CoreError&) = delete;
  CoreError& operator=(CoreError&&) = delete;

  bool m_had_error{false};
  static void error(const std::string& msg);
  static CoreError& getInstance();
};

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class CmdError {
 private:
  CmdError() = default;
  // NOLINTNEXTLINE(readability-identifier-naming)
  void _error(TokenType where, const std::string& msg, CmdErrorType type);

 public:
  CmdError(CmdError& other) = delete;
  CmdError& operator=(const CmdError& other) = delete;
  CmdError(CmdError&& other) = delete;
  CmdError& operator=(CmdError&& other) = delete;

  bool m_had_error{false};
  static void error(TokenType where, const std::string& msg, CmdErrorType type);
  static CmdError& getInstance();
};

template <>
struct std::formatter<CmdErrorType> : std::formatter<std::string_view> {
  constexpr auto format(CmdErrorType type, auto& ctx) const {
    const auto str = [] {
      std::map<CmdErrorType, std::string> res;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INSERT_ELEM(p) res.emplace(p, #p);
      INSERT_ELEM(CmdErrorType::PARSE_ERROR);
      INSERT_ELEM(CmdErrorType::SCAN_ERROR);
      INSERT_ELEM(CmdErrorType::RUNTIME_ERROR);
#undef INSERT_ELEM

      return res;
    };

    return std::formatter<std::string_view>::format(str()[type], ctx);
  };
};

#endif
