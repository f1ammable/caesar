#ifndef TOKEN_H
#define TOKEN_H
#include <map>
#include <string>

#include "object.hpp"
#include "formatter.hpp"

enum class TokenType {
  LEFT_PAREN,
  RIGHT_PAREN,
  MINUS,
  PLUS,
  SLASH,
  STAR,
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  IDENTIFIER,
  STRING,
  NUMBER,
  NIL,
  TRUE,
  FALSE,
  VAR,
  END,
};

template <>
struct std::formatter<TokenType> : std::formatter<string_view> {
  auto format(const TokenType& t, auto& ctx) const {
    const auto strings = [] {
      std::map<TokenType, std::string> res;
#define INSERT_ELEM(p) res.emplace(p, #p);
      INSERT_ELEM(TokenType::LEFT_PAREN);
      INSERT_ELEM(TokenType::RIGHT_PAREN);
      INSERT_ELEM(TokenType::MINUS);
      INSERT_ELEM(TokenType::PLUS);
      INSERT_ELEM(TokenType::SLASH);
      INSERT_ELEM(TokenType::STAR);
      INSERT_ELEM(TokenType::BANG);
      INSERT_ELEM(TokenType::BANG_EQUAL);
      INSERT_ELEM(TokenType::EQUAL);
      INSERT_ELEM(TokenType::EQUAL_EQUAL);
      INSERT_ELEM(TokenType::GREATER);
      INSERT_ELEM(TokenType::GREATER_EQUAL);
      INSERT_ELEM(TokenType::LESS);
      INSERT_ELEM(TokenType::LESS_EQUAL);
      INSERT_ELEM(TokenType::IDENTIFIER);
      INSERT_ELEM(TokenType::STRING);
      INSERT_ELEM(TokenType::NUMBER);
      INSERT_ELEM(TokenType::NIL);
      INSERT_ELEM(TokenType::TRUE);
      INSERT_ELEM(TokenType::FALSE);
      INSERT_ELEM(TokenType::VAR);
      INSERT_ELEM(TokenType::END);
#undef INSERT_ELEM
      return res;
    };

    return std::format_to(ctx.out(), "{}", strings()[t]);
  }
};

class Token {
 public:
  const TokenType m_type;
  const std::string m_lexeme;
  const Object m_literal;
  const int m_line;

  template <typename T>
    requires std::constructible_from<Object, T>
  Token(TokenType type, std::string lexeme, T&& literal, int line)
      : m_type(type),
        m_lexeme(std::move(lexeme)),
        m_literal(std::forward<T>(literal)),
        m_line(line){};
  [[nodiscard]] constexpr std::string toString() const {
    return std::format("{} {} {}", m_type, m_lexeme, m_literal);
  }
};

template <>
struct std::formatter<Token> {
  constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

  auto format(const Token& t, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "{}", t.toString());
  }
};

#endif
