#include "scanner.hpp"

#include <string>
#include <utility>
#include <variant>

#include "error.hpp"
#include "stdlib.hpp"
#include "token.hpp"
#include "util.hpp"

Scanner::Scanner(std::string source) : m_source(std::move(source)) {}

std::vector<Token> Scanner::scanTokens() {
  while (!isAtEnd()) {
    m_start = m_current;
    scanToken();
  }

  m_tokens.emplace_back(TokenType::END, "", std::monostate{}, m_line);
  return m_tokens;
}

bool Scanner::isAtEnd() const { return m_current >= m_source.length(); }

void Scanner::scanToken() {
  char c = advance();
  switch (c) {
    case '(':
      addToken(TokenType::LEFT_PAREN);
      break;
    case ')':
      addToken(TokenType::RIGHT_PAREN);
      break;
    case '{':
      addToken(TokenType::LEFT_BRACE);
      break;
    case '}':
      addToken(TokenType::RIGHT_BRACE);
      break;
    case ',':
      addToken(TokenType::COMMA);
      break;
    case '.':
      addToken(TokenType::DOT);
      break;
    case '-':
      addToken(TokenType::MINUS);
      break;
    case '+':
      addToken(TokenType::PLUS);
      break;
    case ';':
      addToken(TokenType::SEMICOLON);
      break;
    case '*':
      addToken(TokenType::STAR);
      break;
    case '!':
      addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
      break;
    case '=':
      addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
      break;
    case '<':
      addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
      break;
    case '>':
      addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
      break;
    case '/':
      if (match('/')) {
        while (peek() != '\n' && !isAtEnd()) {
          advance();
        }
      } else {
        addToken(TokenType::SLASH);
      }
      break;
    case ' ':
    case '\r':
    case '\t':
      break;
    case '\n':
      m_line++;
      break;
    case '"':
      string();
      break;
    default:
      if (isdigit(c)) {
        number();
      } else if (isalpha(c)) {
        identifier();
      } else {
        e.error(m_line, "Unexpected character");
      }
      break;
  }
}

char Scanner::advance() { return m_source[m_current++]; }

void Scanner::addToken(TokenType tokenType) {
  addToken(tokenType, std::monostate{});
}

bool Scanner::match(const char expected) {
  if (isAtEnd()) return false;
  if (m_source[m_current] != expected) return false;

  m_current++;
  return true;
}

char Scanner::peek() const {
  if (isAtEnd()) return '\0';
  return m_source[m_current];
}

void Scanner::string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') {
      m_line++;
    }
    advance();
  }

  if (isAtEnd()) {
    e.error(m_line, "Unterminated string");
    return;
  }

  advance();

  std::string value = m_source.substr(m_start + 1, m_current - m_start - 2);
  addToken(TokenType::STRING, value.data());
}

void Scanner::number() {
  while (isdigit(peek())) {
    advance();
  }

  if (peek() == '.' && isdigit(peekNext())) {
    advance();

    while (isdigit(peek())) {
      advance();
    }
  }

  std::string text = m_source.substr(m_start, m_current - m_start);
  auto value = detail::parseNumber(text);

  addToken(TokenType::NUMBER, value);
}

char Scanner::peekNext() const {
  if (m_current + 1 >= m_source.length()) {
    return '\0';
  }
  return m_source[m_current + 1];
}

void Scanner::identifier() {
  while (isalnum(peek())) advance();

  const std::string text = m_source.substr(m_start, m_current - m_start);
  TokenType type;
  try {
    type = m_keywords.at(text);
  } catch (std::out_of_range& e) {
    type = TokenType::IDENTIFIER;
  }

  addToken(type);
}

