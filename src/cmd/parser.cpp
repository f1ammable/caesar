#include "parser.hpp"

#include <memory>
#include <variant>

#include "cmd/token_type.hpp"
#include "error.hpp"
#include "expr.hpp"
#include "stmnt.hpp"
#include "token.hpp"

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::equality() {
  std::unique_ptr<Expr> expr = comparison();

  while (match(TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL)) {
    const Token op = previous();
    std::unique_ptr<Expr> right = comparison();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

bool Parser::check(TokenType type) { return peek().m_type == type; }

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
    const Token op = previous();
    std::unique_ptr<Expr> right = factor();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  std::unique_ptr<Expr> expr = unary();

  while (match(TokenType::SLASH, TokenType::STAR)) {
    const Token op = previous();
    std::unique_ptr<Expr> right = unary();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match(TokenType::BANG, TokenType::MINUS)) {
    const Token op = previous();
    std::unique_ptr<Expr> right = unary();
    return std::make_unique<Unary>(op, std::move(right));
  }
  return primary();
}

std::unique_ptr<Expr> Parser::primary() {
  if (match(TokenType::BOOL_FALSE)) return std::make_unique<Literal>(false);
  if (match(TokenType::BOOL_TRUE)) return std::make_unique<Literal>(true);
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

  CmdError::error(peek().m_type, "Expected expression.",
                  CmdErrorType::PARSE_ERROR);
  return nullptr;
}

std::unique_ptr<Expr> Parser::comparison() {
  std::unique_ptr<Expr> expr = term();

  while (match(TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS,
               TokenType::LESS_EQUAL)) {
    const Token op = previous();
    std::unique_ptr<Expr> right = term();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

Token Parser::consume(TokenType type, const std::string& msg) {
  if (check(type)) return advance();

  CmdError::error(peek().m_type, msg, CmdErrorType::PARSE_ERROR);
  return peek();  // Return current token on error
}

std::unique_ptr<Stmnt> Parser::statement() {
  if (check(TokenType::IDENTIFIER)) {
    const int saved = m_current;
    advance();

    // Assignment
    if (check(TokenType::EQUAL)) {
      m_current = saved;
      return exprStmnt();
    }

    // Function call
    if (check(TokenType::IDENTIFIER) || check(TokenType::NUMBER) ||
        check(TokenType::STRING) || check(TokenType::LEFT_PAREN) ||
        check(TokenType::BOOL_TRUE) || check(TokenType::BOOL_FALSE) ||
        check(TokenType::NIL) || check(TokenType::END)) {
      m_current = saved;
      return funStmnt();
    }

    // Expression
    m_current = saved;
  }

  return exprStmnt();
}

std::unique_ptr<Stmnt> Parser::exprStmnt() {
  std::unique_ptr<Expr> expr = expression();
  if (!expr) return nullptr;  // Don't continue if expression failed
  consume(TokenType::END, "Expect EOF after expression.");
  return std::make_unique<ExprStmnt>(std::move(expr));
}

std::unique_ptr<Stmnt> Parser::parse() { return declaration(); }

std::unique_ptr<Stmnt> Parser::declaration() {
  if (match(TokenType::VAR)) return varDeclaration();
  return statement();
}

std::unique_ptr<Stmnt> Parser::varDeclaration() {
  const Token name = consume(TokenType::IDENTIFIER, "Expect variable name");
  std::unique_ptr<Expr> initialiser = {};
  if (match(TokenType::EQUAL)) {
    initialiser = expression();
    if (!initialiser) return nullptr;  // Don't continue if expression failed
  }
  consume(TokenType::END, "Expect EOF after variable declaration");
  return std::make_unique<VarStmnt>(name, std::move(initialiser));
}

std::unique_ptr<Expr> Parser::assignment() {
  std::unique_ptr<Expr> expr = equality();

  if (match(TokenType::EQUAL)) {
    const Token equals = previous();
    std::unique_ptr<Expr> value = assignment();

    if (auto* ptr = dynamic_cast<Variable*>(expr.get())) {
      const Token name = ptr->m_name;
      return std::make_unique<Assign>(name, std::move(value));
    }

    CmdError::error(equals.m_type, "Invalid assignment target.",
                    CmdErrorType::PARSE_ERROR);
  }

  return expr;
}

std::unique_ptr<Stmnt> Parser::funStmnt() {
  const Token fnName = consume(TokenType::IDENTIFIER, "Expect function name");

  std::vector<std::unique_ptr<Expr>> args = {};

  while (!check(TokenType::END)) {
    auto arg = expression();
    if (!arg) return nullptr;  // Don't continue if expression failed
    args.emplace_back(std::move(arg));
  }

  consume(TokenType::END, "Expect EOF after function call.");
  return std::make_unique<CallStmnt>(fnName, std::move(args));
}
