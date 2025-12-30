#ifndef CAESAR_TEST_HELPERS_HPP
#define CAESAR_TEST_HELPERS_HPP

#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <variant>
#include <vector>

#include "expr.hpp"
#include "parser.hpp"
#include "scanner.hpp"
#include "stmnt.hpp"
#include "token.hpp"
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
inline EType* getTopExpr(const std::unique_ptr<Stmnt>& p_stmnt) {
  auto stmnt = dynamic_cast<ExprStmnt*>(p_stmnt.get());
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
  AutoRestoreRdbuf(const AutoRestoreRdbuf&) = delete;
  AutoRestoreRdbuf(AutoRestoreRdbuf&&) = delete;

  AutoRestoreRdbuf(std::ostream& out) : out(out), old(out.rdbuf()) {}
};

template <typename Return>
inline std::string captureStream(std::function<Return(const std::string&)> fn,
                                 const std::string& in,
                                 std::ostream& out = std::cerr) {
  AutoRestoreRdbuf restore{std::cerr};
  std::ostringstream oss;
  std::cerr.rdbuf(oss.rdbuf());
  fn(in);
  return oss.str();
}
}  // namespace helpers

#endif
