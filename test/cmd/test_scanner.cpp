#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cmd/token.hpp>

#include "test_helpers.hpp"

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
    captured = helpers::captureStream(std::cerr, helpers::scan, input);
  } catch (...) {
    REQUIRE(captured == error_message);
  }
}

TEST_CASE("Test register variable tokenisation", "[scanner]") {
  auto [input, expected_lexeme] =
      GENERATE(table<std::string, std::string>({{"$pc", "$pc"},
                                                {"$rax", "$rax"},
                                                {"$sp", "$sp"},
                                                {"$x0", "$x0"}}));

  auto tokens = helpers::scan(input);
  REQUIRE(helpers::checkTokensSize(tokens.size(), 1));
  REQUIRE(tokens[0].m_type == TokenType::IDENTIFIER);
  REQUIRE(tokens[0].m_lexeme == expected_lexeme);
}

TEST_CASE("Test register variable in expression", "[scanner]") {
  auto tokens = helpers::scan("$pc+4");
  REQUIRE(helpers::checkTokensSize(tokens.size(), 3));
  REQUIRE(tokens[0].m_type == TokenType::IDENTIFIER);
  REQUIRE(tokens[0].m_lexeme == "$pc");
  REQUIRE(tokens[1].m_type == TokenType::PLUS);
  REQUIRE(tokens[2].m_type == TokenType::NUMBER);
}
