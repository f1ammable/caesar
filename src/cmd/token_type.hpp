#ifndef CAESAR_TOKEN_TYPE_H
#define CAESAR_TOKEN_TYPE_H

#include <cstdint>

enum class TokenType : std::uint8_t {
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
  BOOL_TRUE,
  BOOL_FALSE,
  VAR,
  END,
};

#endif
