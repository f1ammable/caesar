#ifndef PARSER_HPP
#define PARSER_HPP

#include "expr.hpp"
#include "parse_error.hpp"
#include "stmnt.hpp"
#include "token.hpp"
#include <vector>
class Parser {
private:
  std::vector<Token> m_tokens;
  int m_current = 0;

  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> equality();
  template <typename... Args>
    requires(std::is_same_v<TokenType, Args> && ...)
  bool match(Args... types) {
    return (... || (check(types) ? (advance(), true) : false));
  }
  bool check(TokenType type);
  Token advance();
  bool isAtEnd();
  Token peek();
  Token previous();
  std::unique_ptr<Expr> term();
  std::unique_ptr<Expr> factor();
  std::unique_ptr<Expr> unary();
  std::unique_ptr<Expr> primary();
  std::unique_ptr<Expr> comparison();
  Token consume(TokenType type, const std::string &msg);
  ParseError error(Token token, const std::string &msg);
  void synchronise();
  std::unique_ptr<Stmnt> statement();
  std::unique_ptr<Stmnt> printStmnt();
  std::unique_ptr<Stmnt> exprStmnt();

public:
  explicit Parser(std::vector<Token> tokens);
  std::vector<std::unique_ptr<Stmnt>> parse();
};

#endif // !PARSER_HPP
