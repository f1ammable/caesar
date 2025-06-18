#include "scanner.hpp"

#include "fmt/format.h"

void Scanner::initToken() {
  m_token = std::make_shared<Token>("", m_cs.currentRow(), m_cs.currentCol(),
                                    Token::None);
}

void Scanner::setText(const std::string &text) { m_token->value = text; }

void Scanner::setType(const int type) { m_token->type = type; }

void Scanner::enterChar() { m_token->value.append(1, currentChar()); }

bool Scanner::intLiteral() {
  if (!isdigit(currentChar())) {
    return false;
  }

  while (isdigit(currentChar())) {
    enterChar();
    nextChar();
  }
  return true;
}

bool Scanner::numLiteral() {
  setType(Token::Number);

  if (isdigit(currentChar())) {
    intLiteral();
    if (currentChar() == '.') {
      enterChar();
      nextChar();
      if (isdigit(currentChar())) {
        intLiteral();
      }
    }
  } else if (currentChar() == '.') {
    enterChar();
    nextChar();
    if (!intLiteral()) {
      return false;
    }
  } else {
    return false;
  }
  return true;
}

Scanner::Scanner(CharStream &cs) : m_cs(cs) { nextToken(); }

std::shared_ptr<Token> Scanner::currentToken() const { return m_token; }

std::shared_ptr<Token> Scanner::nextToken() {
  skipWhitespace();
  initToken();

  if (char c; (c = currentChar()) != EOF) {
    switch (c) {
      case '+':
        setType(Token::Operator);
        enterChar();
        nextChar();
        break;
      case '-':
        setType(Token::Operator);
        enterChar();
        nextChar();
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '.':
        numLiteral();
        break;
      default:
        throw "Invalid char!";
    }
  } else {
    setType(EOF);
    setText("EOF");
  }
  std::cout << fmt::format("Scanning token: {} with type {}\n", m_token->value,
                           m_token->type);
  return m_token;
}

char Scanner::currentChar() const { return m_cs.currentChar(); }

char Scanner::nextChar() { return m_cs.nextChar(); }

bool Scanner::isWhitespace(char c) const {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

void Scanner::skipWhitespace() {
  while (isWhitespace(currentChar())) {
    nextChar();
  }
}
