#ifndef SCANNER_H
#define SCANNER_H
#include <map>
#include <string>
#include <vector>

#include "error.hpp"
#include "token.hpp"

class Scanner {
 private:
  Err& e = Err::getInstance();
  const std::string m_source;
  std::vector<Token> m_tokens;
  int m_start = 0;
  int m_current = 0;
  int m_line = 1;
  std::map<std::string, TokenType> m_keywords = {
      {"and", TokenType::AND},     {"class", TokenType::CLASS},
      {"else", TokenType::ELSE},   {"false", TokenType::FALSE},
      {"for", TokenType::FOR},     {"fun", TokenType::FUN},
      {"if", TokenType::IF},       {"nil", TokenType::NIL},
      {"or", TokenType::OR},       {"return", TokenType::RETURN},
      {"super", TokenType::SUPER}, {"this", TokenType::THIS},
      {"true", TokenType::TRUE},   {"var", TokenType::VAR},
      {"while", TokenType::WHILE}};

  [[nodiscard]] bool isAtEnd() const;
  void scanToken();
  char advance();
  void addToken(TokenType tokenType);
  template <typename T>
    requires std::constructible_from<Object, T>
  void addToken(TokenType tokenType, const T& literal) {
    const std::string text = m_source.substr(m_start, m_current - m_start);
    m_tokens.emplace_back(tokenType, text, literal, m_line);
  }
  bool match(char expected);
  [[nodiscard]] char peek() const;
  void string();
  void number();
  [[nodiscard]] char peekNext() const;
  void identifier();

 public:
  explicit Scanner(std::string source);
  std::vector<Token> scanTokens();
};

#endif
