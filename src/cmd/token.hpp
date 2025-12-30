#ifndef TOKEN_H
#define TOKEN_H
#include <string>

#include "object.hpp"
#include "token_type.hpp"

class Token {
 public:
  const TokenType m_type;
  const std::string m_lexeme;
  const Object m_literal;

  template <typename T>
    requires std::constructible_from<Object, T>
  Token(TokenType type, std::string lexeme, T&& literal)
      : m_type(type),
        m_lexeme(std::move(lexeme)),
        m_literal(std::forward<T>(literal)) {}
  [[nodiscard]] std::string toString() const;
};

#endif
