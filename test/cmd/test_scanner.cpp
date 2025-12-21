#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <iostream>
#include <ostream>
#include <sstream>

#include "scanner.hpp"
#include "token.hpp"

namespace helpers {
// Pass here expected amount of tokens WITHOUT end token
inline bool checkTokensSize(const size_t actual, const size_t expected) {
  return actual == expected + 1;
}

inline bool checkTokenType(const Token& token, const TokenType& expectedType) {
  return token.m_type == expectedType;
}

inline std::vector<Token> scan(const std::string& src) {
  Scanner s(src);
  auto tokens = s.scanTokens();
  REQUIRE(!tokens.empty());
  REQUIRE(tokens.back().m_type == TokenType::END);
  return tokens;
}

class AutoRestoreRdbuf {
  std::ostream& out;
  std::streambuf* old;

 public:
  ~AutoRestoreRdbuf() { out.rdbuf(old); }
  AutoRestoreRdbuf(const AutoRestoreRdbuf&) = delete;
  AutoRestoreRdbuf(AutoRestoreRdbuf&&) = delete;

  AutoRestoreRdbuf(std::ostream& out) : out(out), old(out.rdbuf()) {}
};

inline std::string captureStream(
    std::function<std::vector<Token>(const std::string&)> fn,
    const std::string& in, std::ostream& out = std::cout) {
  AutoRestoreRdbuf restore{std::cout};
  std::ostringstream oss;
  std::cout.rdbuf(oss.rdbuf());
  fn(in);
  return oss.str();
}
}  // namespace helpers

TEST_CASE("Test single-character tokenisation", "[scanner]") {
  auto [input, expected_type] =
      GENERATE(table<std::string, TokenType>({{"(", TokenType::LEFT_PAREN},
                                              {")", TokenType::RIGHT_PAREN},
                                              {"-", TokenType::MINUS},
                                              {"+", TokenType::PLUS},
                                              {"*", TokenType::STAR},
                                              {"!", TokenType::BANG},
                                              {"=", TokenType::EQUAL},
                                              {"<", TokenType::LESS},
                                              {">", TokenType::GREATER},
                                              {"/", TokenType::SLASH}}));

  auto tokens = helpers::scan(input);
  REQUIRE(helpers::checkTokensSize(tokens.size(), 1));
  REQUIRE(tokens[0].m_type == expected_type);
}

TEST_CASE("Test two-character tokenisation", "[scanner]") {
  auto [input, expected_type] = GENERATE(
      table<std::string, TokenType>({{"!=", TokenType::BANG_EQUAL},
                                     {"==", TokenType::EQUAL_EQUAL},
                                     {"<=", TokenType::LESS_EQUAL},
                                     {">=", TokenType::GREATER_EQUAL}}));

  auto tokens = helpers::scan(input);
  REQUIRE(helpers::checkTokensSize(tokens.size(), 1));
  REQUIRE(tokens[0].m_type == expected_type);
}

TEST_CASE("Test single-char vs two-char operators", "[scanner]") {
  auto [input, first_type, second_type] =
      GENERATE(table<std::string, TokenType, TokenType>(
          {{"! =", TokenType::BANG, TokenType::EQUAL},
           {"= =", TokenType::EQUAL, TokenType::EQUAL},
           {"< =", TokenType::LESS, TokenType::EQUAL},
           {"> =", TokenType::GREATER, TokenType::EQUAL}}));

  auto tokens = helpers::scan(input);
  REQUIRE(helpers::checkTokensSize(tokens.size(), 2));
  REQUIRE(tokens[0].m_type == first_type);
  REQUIRE(tokens[1].m_type == second_type);
}

TEST_CASE("Test string and number tokenisation", "[scanner]") {
  auto [input, expected_type] = GENERATE(
      table<std::string, TokenType>({{"\"hello\"", TokenType::STRING},
                                     {"5", TokenType::NUMBER},
                                     {"5.10", TokenType::NUMBER},
                                     {"55", TokenType::NUMBER},
                                     {"varName", TokenType::IDENTIFIER}}));

  auto tokens = helpers::scan(input);
  REQUIRE(helpers::checkTokensSize(tokens.size(), 1));
  REQUIRE(tokens[0].m_type == expected_type);
}

TEST_CASE("Test errors on malformed input", "[scanner]") {
  auto [input, error_message] = GENERATE(table<std::string, std::string>(
      {{"12.", "Error : Unexpected character"},
       {"\"string", "Error : Unterminated string"}}));

  std::string captured{};
  try {
    captured = helpers::captureStream(&helpers::scan, input);
  } catch (...) {
    REQUIRE(captured == error_message);
  }
}
