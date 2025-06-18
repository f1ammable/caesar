#ifndef CAESAR_AST_HPP
#define CAESAR_AST_HPP

#include <memory>
#include <vector>

#include "token.hpp"
#include "visitor.hpp"

class AbstractNode {
 public:
  ABSTRACT_ACCEPT_METHOD_DECL

  std::shared_ptr<Token> token;
  std::vector<std::shared_ptr<AbstractNode>> children;

  explicit AbstractNode(std::shared_ptr<Token> token);
  virtual ~AbstractNode();
  void addChild(const std::shared_ptr<AbstractNode> &child);
};

class BinaryExpr final : public AbstractNode {
 public:
  CONCRETE_ACCEPT_METHOD_DECL

  explicit BinaryExpr(const std::shared_ptr<Token> &token);
  ~BinaryExpr() override {}
};

class NumLiteral final : public AbstractNode {
 public:
  CONCRETE_ACCEPT_METHOD_DECL

  explicit NumLiteral(const std::shared_ptr<Token> &token);
  ~NumLiteral() override {}
};

#endif