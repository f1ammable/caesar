#include "parser.hpp"

#include <memory>
#include <variant>

#include "error.hpp"
#include "expr.hpp"
#include "parse_error.hpp"
#include "stmnt.hpp"
#include "token.hpp"

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::equality() {
  std::unique_ptr<Expr> expr = comparison();

  while (match(TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL)) {
    Token op = previous();
    std::unique_ptr<Expr> right = comparison();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

bool Parser::check(TokenType type) {
  if (isAtEnd()) return false;
  return peek().m_type == type;
}

Token Parser::advance() {
  if (!isAtEnd()) m_current++;
  return previous();
}

bool Parser::isAtEnd() { return peek().m_type == TokenType::END; }

Token Parser::peek() { return m_tokens[m_current]; }

Token Parser::previous() { return m_tokens[m_current - 1]; }

std::unique_ptr<Expr> Parser::term() {
  std::unique_ptr<Expr> expr = factor();

  while (match(TokenType::MINUS, TokenType::PLUS)) {
    Token op = previous();
    std::unique_ptr<Expr> right = factor();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  std::unique_ptr<Expr> expr = unary();

  while (match(TokenType::SLASH, TokenType::STAR)) {
    Token op = previous();
    std::unique_ptr<Expr> right = unary();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match(TokenType::BANG, TokenType::MINUS)) {
    Token op = previous();
    std::unique_ptr<Expr> right = unary();
    return std::make_unique<Unary>(op, std::move(right));
  }
  return primary();
}

std::unique_ptr<Expr> Parser::primary() {
  if (match(TokenType::FALSE)) return std::make_unique<Literal>(false);
  if (match(TokenType::TRUE)) return std::make_unique<Literal>(true);
  if (match(TokenType::NIL)) return std::make_unique<Literal>(std::monostate{});

  if (match(TokenType::NUMBER, TokenType::STRING)) {
    return std::make_unique<Literal>(previous().m_literal);
  }

  if (match(TokenType::IDENTIFIER)) {
    return std::make_unique<Variable>(previous());
  }

  if (match(TokenType::LEFT_PAREN)) {
    std::unique_ptr<Expr> e = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    return std::make_unique<Grouping>(std::move(e));
  }

  throw error(peek(), "Expected expression.");
}

std::unique_ptr<Expr> Parser::comparison() {
  std::unique_ptr<Expr> expr = term();

  while (match(TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS,
               TokenType::LESS_EQUAL)) {
    Token op = previous();
    std::unique_ptr<Expr> right = term();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

Token Parser::consume(TokenType type, const std::string& msg) {
  if (check(type)) return advance();

  throw error(peek(), msg);
}

ParseError Parser::error(Token token, const std::string& msg) {
  Err& err = Err::getInstance();
  err.error(token, msg);
  return ParseError();
}

void Parser::synchronise() {
  advance();

  while (!isAtEnd()) {
    if (previous().m_type == TokenType::SEMICOLON) return;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    switch (peek().m_type) {
      case TokenType::CLASS:
      case TokenType::FUN:
      case TokenType::VAR:
      case TokenType::FOR:
      case TokenType::IF:
      case TokenType::WHILE:
      case TokenType::PRINT:
      case TokenType::RETURN:
        return;
    }
#pragma clang diagnostic pop

    advance();
  }
}

std::unique_ptr<Stmnt> Parser::statement() {
  if (check(TokenType::IDENTIFIER)) {
    int saved = m_current;
    advance(); 
    if (!check(TokenType::SEMICOLON) && !check(TokenType::EQUAL)) {
      m_current = saved;
      return funStmnt();
    }
    m_current = saved;
  }

  return exprStmnt();
}

std::unique_ptr<Stmnt> Parser::printStmnt() {
  std::unique_ptr<Expr> value = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  return std::make_unique<PrintStmnt>(std::move(value));
}

std::unique_ptr<Stmnt> Parser::exprStmnt() {
  std::unique_ptr<Expr> expr = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  return std::make_unique<ExprStmnt>(std::move(expr));
}

std::vector<std::unique_ptr<Stmnt>> Parser::parse() {
  std::vector<std::unique_ptr<Stmnt>> statements;

  while (!isAtEnd()) {
    statements.emplace_back(declaration());
  }

  return statements;
}

std::unique_ptr<Stmnt> Parser::declaration() {
  try {
    if (match(TokenType::VAR)) return varDeclaration();

    return statement();
  } catch (ParseError& err) {
    synchronise();
    // TODO: Maybe this isn't the _best_ idea
    return nullptr;
  }
}

std::unique_ptr<Stmnt> Parser::varDeclaration() {
  Token name = consume(TokenType::IDENTIFIER, "Expect variable name");
  std::unique_ptr<Expr> initialiser = {};
  if (match(TokenType::EQUAL)) initialiser = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");
  return std::make_unique<VarStmnt>(name, std::move(initialiser));
}

std::unique_ptr<Expr> Parser::assignment() {
  std::unique_ptr<Expr> expr = equality();

  if (match(TokenType::EQUAL)) {
    Token equals = previous();
    std::unique_ptr<Expr> value = assignment();

    if (auto* ptr = dynamic_cast<Variable*>(value.get())) {
      Token name = ptr->m_name;
      return std::make_unique<Assign>(name, std::move(value));
    }

    error(equals, "Invalid assignment target.");
  }

  return expr;
}

std::unique_ptr<Stmnt> Parser::funStmnt() {
  Token fnName = consume(TokenType::IDENTIFIER, "Expect function name");

  std::vector<std::unique_ptr<Expr>> args = {};

  while (!check(TokenType::SEMICOLON)) {
    args.emplace_back(primary());
  }

  consume(TokenType::SEMICOLON, "Expect ';' after function call.");
  return std::make_unique<CallStmnt>(fnName, std::move(args));
}
