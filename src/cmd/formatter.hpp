#ifndef FORMATTER_H
#define FORMATTER_H

#include <format>
#include <map>
#include <memory>
#include <string>
#include <variant>

#include "callable.hpp"
#include "object.hpp"
#include "token.hpp"
#include "token_type.hpp"

namespace std {
template <>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct formatter<Object> {
  static constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  static auto format(const Object& obj, format_context& ctx) {
    auto visitor = [&ctx](const auto& value) -> format_context::iterator {
      using T = decay_t<decltype(value)>;

      if constexpr (is_same_v<T, monostate>) {
        return format_to(ctx.out(), "(null)");
      } else if constexpr (is_same_v<T, bool>) {
        return format_to(ctx.out(), "{}", value ? "true" : "false");
      } else if constexpr (is_same_v<T, double>) {
        string text = std::format("{}", value);

        if (text.ends_with(".0")) {
          text = text.substr(0, text.length() - 2);
        }

        return format_to(ctx.out(), "{}", text);

      } else if constexpr (is_same_v<T, shared_ptr<Callable>>) {
        return format_to(ctx.out(), "{}", value->str());
      }

      else {
        return format_to(ctx.out(), "{}", value);
      }
    };

    return visit(visitor, obj);
  }
};

template <>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct formatter<Token> {
  static constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  static auto format(const Token& t, format_context& ctx) {
    return format_to(ctx.out(), "{}", t.toString());
  }
};

template <>
// NOLINTNEXTLINE(cert-dcl58-cpp)
struct formatter<TokenType> {
  static constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  static auto format(const TokenType& t, format_context& ctx) {
    const auto str = [] {
      map<TokenType, string> res;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
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

    return format_to(ctx.out(), "{}", str()[t]);
  }
};
}  // namespace std

inline std::string Token::toString() const {
  return std::format("{} {} {}", m_type, m_lexeme, m_literal);
}

#endif
