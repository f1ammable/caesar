#include "ast.hpp"

#include "fmt/format.h"

AbstractNode::AbstractNode(std::shared_ptr<Token> token)
    : token(std::move(token)) {
  std::cout << fmt::format("Creating AST node: {}\n", token->value);
}

AbstractNode::~AbstractNode() {
  std::cout << fmt::format("Destroying AST node: {}\n", token->value);
}

void AbstractNode::addChild(const std::shared_ptr<AbstractNode> &child) {
  children.push_back(child);
}

BinaryExpr::BinaryExpr(const std::shared_ptr<Token> &token)
    : AbstractNode(token) {}

NumLiteral::NumLiteral(const std::shared_ptr<Token> &token)
    : AbstractNode(token) {}

CONCRETE_ACCEPT_METHOD_IMPL(BinaryExpr);
CONCRETE_ACCEPT_METHOD_IMPL(NumLiteral);