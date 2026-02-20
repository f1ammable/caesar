#ifndef SCANNER_H
#define SCANNER_H
#include <map>
#include <string>
#include <vector>

#include "error.hpp"
#include "token.hpp"

class Scanner {
 private:
  const std::string m_source;
  std::vector<Token> m_tokens;
  std::map<std::string, TokenType> m_keywords = {{"false", TokenType::BOOL_FALSE},
                                                 {"nil", TokenType::NIL},
                                                 {"true", TokenType::BOOL_TRUE},
                                                 {"var", TokenType::VAR}};
  int m_start = 0;
  int m_current = 0;

  [[nodiscard]] bool isAtEnd() const;
  void scanToken();
  char advance();
  void addToken(TokenType tokenType);
  template <typename T>
    requires std::constructible_from<Object, T>
  void addToken(TokenType tokenType, const T& literal) {
    const std::string text = m_source.substr(m_start, m_current - m_start);
    m_tokens.emplace_back(tokenType, text, literal);
  }
  bool match(char expected);
  [[nodiscard]] char peek() const;
  void string();
  void number();
  [[nodiscard]] char peekNext() const;
  void identifier();

 public:
  explicit Scanner(std::string source) noexcept;
  std::vector<Token> scanTokens();
};

#endif
