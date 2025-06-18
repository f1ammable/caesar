#include "parser.hpp"

std::shared_ptr<Token> Parser::currentToken() const {
  return m_scanner.currentToken();
}

std::shared_ptr<Token> Parser::nextToken() { return m_scanner.nextToken(); }

bool Parser::match(const std::shared_ptr<Token>& token, const int type) {
  if (token->type != type) {
    return false;
  }
  nextToken();
  return true;
}

Parser::Parser(const Scanner& scanner) : m_scanner(std::move(scanner)) {}

std::shared_ptr<AbstractNode> Parser::expr() {
  auto root = num();

  while (currentToken()->type == Token::Operator) {
    auto token = currentToken();
    nextToken();
    auto lhs = root;
    auto rhs = num();
    root = std::make_shared<BinaryExpr>(token);
    root->addChild(lhs);
    root->addChild(rhs);
  }

  if (currentToken()->type == EOF) {
    std::cout << "accepted!\n";
  } else {
    throw "Syntax Error: Unexpected token!";
  }
  return root;
}

std::shared_ptr<AbstractNode> Parser::num() {
  if (auto token = currentToken(); match(token, Token::Number)) {
    return std::make_shared<NumLiteral>(token);
  }
  throw "Syntax Error: Number expected!";
}
