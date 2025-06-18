#ifndef CAESAR_PARSER_HPP
#define CAESAR_PARSER_HPP

#include "ast.hpp"
#include "scanner.hpp"

class Parser {
 private:
  Scanner m_scanner;

  std::shared_ptr<Token> currentToken() const;
  std::shared_ptr<Token> nextToken();
  bool match(const std::shared_ptr<Token>& token, int type);

 public:
  explicit Parser(const Scanner& scanner);

  std::shared_ptr<AbstractNode> expr();
  std::shared_ptr<AbstractNode> num();
};

#endif