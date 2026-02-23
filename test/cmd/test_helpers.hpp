#ifndef CAESAR_TEST_HELPERS_HPP
#define CAESAR_TEST_HELPERS_HPP

#include <catch2/catch_test_macros.hpp>
#include <cmd/expr.hpp>
#include <cmd/parser.hpp>
#include <cmd/scanner.hpp>
#include <cmd/stmnt.hpp>
#include <cmd/token.hpp>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <utility>
#include <variant>
#include <vector>

namespace helpers {
// Pass here expected amount of tokens WITHOUT end token
inline bool checkTokensSize(const size_t actual, const size_t expected) {
  return actual == expected + 1;
}

inline std::vector<Token> scan(const std::string& src) {
  Scanner s(src);
  auto tokens = s.scanTokens();
  REQUIRE(!tokens.empty());
  REQUIRE(tokens.back().m_type == TokenType::END);
  return tokens;
}

inline std::unique_ptr<Stmnt> getStmnt(const std::string& input) {
  auto tokens = scan(input);
  Parser p(std::move(tokens));
  return p.parse();
}

template <typename EType>
inline EType* getTopExpr(const std::unique_ptr<Stmnt>& pStmnt) {
  auto* stmnt = dynamic_cast<ExprStmnt*>(pStmnt.get());
  REQUIRE(stmnt != nullptr);

  auto expr = dynamic_cast<EType*>(stmnt->m_expr.get());
  REQUIRE(expr != nullptr);
  return expr;
}

template <typename StmntType, typename ExprType, typename Expected>
inline void checkStmntTypeAndValue(const std::unique_ptr<Stmnt>& pStmnt,
                                   const Expected& expected) {
  ExprType* expr = nullptr;
  auto stmnt = dynamic_cast<StmntType*>(pStmnt.get());
  REQUIRE(stmnt != nullptr);

  if constexpr (std::same_as<StmntType, ExprStmnt>)
    expr = dynamic_cast<ExprType*>(stmnt->m_expr.get());
  else if constexpr (std::same_as<StmntType, VarStmnt>)
    expr = dynamic_cast<ExprType*>(stmnt->m_initialiser.get());

  if (!expr) FAIL("Could not properly downcast expression type.");

  if constexpr (std::same_as<ExprType, Literal>) {
    REQUIRE(std::holds_alternative<Expected>(expr->m_value));
    auto exprValue = std::get<Expected>(expr->m_value);
    REQUIRE(exprValue == expected);
  }
}

class AutoRestoreRdbuf {
  std::ostream& out;
  std::streambuf* old;

 public:
  ~AutoRestoreRdbuf() { out.rdbuf(old); }
  AutoRestoreRdbuf& operator=(const AutoRestoreRdbuf&) = delete;
  AutoRestoreRdbuf& operator=(AutoRestoreRdbuf&&) = delete;
  AutoRestoreRdbuf(const AutoRestoreRdbuf&) = delete;
  AutoRestoreRdbuf(AutoRestoreRdbuf&&) = delete;

  explicit AutoRestoreRdbuf(std::ostream& out) : out(out), old(out.rdbuf()) {}
};

template <typename Func, typename... Args>
inline std::string captureStream(std::ostream& out, Func&& fn, Args&&... args) {
  AutoRestoreRdbuf restore{out};
  std::ostringstream oss;
  out.rdbuf(oss.rdbuf());
  std::invoke(std::forward<Func>(fn), std::forward<Args>(args)...);
  return oss.str();
}

inline Token makeToken(const std::string& name) {
  return Token(TokenType::IDENTIFIER, name, std::monostate{});
}

// Simple subcommand for testing
inline Object testSubcmd(const std::vector<std::string>& args) {
  return args.empty() ? std::string("ok") : args[0];
}
}  // namespace helpers

#endif
