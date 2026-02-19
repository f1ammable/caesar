#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cmd/expr.hpp>
#include <cmd/object.hpp>
#include <cmd/stmnt.hpp>
#include <cmd/token.hpp>
#include <cstddef>
#include <memory>
#include <variant>

#include "test_helpers.hpp"

TEST_CASE("Test literal parsing", "[parser][expression][literal]") {
  auto [input, expected_value] =
      GENERATE(table<std::string, Object>({{"42", 42.0},
                                           {"3.14", 3.14},
                                           {"0", 0.0},
                                           {"\"hello\"", "hello"},
                                           {"false", false},
                                           {"true", true},
                                           {"nil", std::monostate{}}}));

  auto stmnt = helpers::getStmnt(input);
  auto visitor = [&stmnt](const auto& obj) -> void {
    helpers::checkStmntTypeAndValue<ExprStmnt, Literal>(stmnt, obj);
  };

  std::visit(visitor, expected_value);

  SECTION("Function parsing") {
    auto stmnt = helpers::getStmnt("print 5");
    auto* callStmnt = dynamic_cast<CallStmnt*>(stmnt.get());
    REQUIRE(callStmnt != nullptr);
    REQUIRE(callStmnt->m_fn.m_type == TokenType::IDENTIFIER);
  }
}

TEST_CASE("Test variable parsing", "[parser][variable][assign]") {
  auto [input, expected_value] = GENERATE(table<std::string, Object>({
      {"var x = 5", 5.0},
      {"var x = \"hello\"", "hello"},
      {"var x = true", true},
      {"var x = nil", std::monostate{}},
  }));

  auto stmnt = helpers::getStmnt(input);

  auto visitor = [&stmnt](const auto& obj) -> void {
    helpers::checkStmntTypeAndValue<VarStmnt, Literal>(stmnt, obj);
  };

  std::visit(visitor, expected_value);

  SECTION("Test variable parsing with functions as values") {
    auto stmnt = helpers::getStmnt("var x = print");
    auto* varStmnt = dynamic_cast<VarStmnt*>(stmnt.get());
    REQUIRE(varStmnt != nullptr);
    auto* var = dynamic_cast<Variable*>(varStmnt->m_initialiser.get());
    REQUIRE(var != nullptr);
  }

  SECTION("Variable re-assignment") {
    auto stmnt = helpers::getStmnt("x = 5");
    auto* assign = helpers::getTopExpr<Assign>(stmnt);
    REQUIRE(assign->m_name.m_lexeme == "x");
  }
}

TEST_CASE("Test grouping expression parsing",
          "[parser][expression][grouping]") {
  SECTION("Simple grouping") {
    auto stmnt = helpers::getStmnt("(5)");
    REQUIRE(helpers::getTopExpr<Grouping>(stmnt) != nullptr);
  }

  SECTION("Grouping with operator") {
    auto stmnt = helpers::getStmnt("(5+3)");
    auto* grouping = helpers::getTopExpr<Grouping>(stmnt);
    auto* binary = dynamic_cast<Binary*>(grouping->m_expr.get());

    REQUIRE(binary != nullptr);
    REQUIRE(binary->m_op.m_type == TokenType::PLUS);
  }

  SECTION("Grouping overrides precedence") {
    auto stmnt = helpers::getStmnt("(2+3)*4");
    auto* binary = helpers::getTopExpr<Binary>(stmnt);
    REQUIRE(binary->m_op.m_type == TokenType::STAR);
    auto* grouping = dynamic_cast<Grouping*>(binary->m_left.get());
    REQUIRE(grouping != nullptr);
  }

  SECTION("Grouping on the right side") {
    auto stmnt = helpers::getStmnt("2*(3+4)");
    auto* binary = helpers::getTopExpr<Binary>(stmnt);
    REQUIRE(binary->m_op.m_type == TokenType::STAR);
    auto* grouping = dynamic_cast<Grouping*>(binary->m_right.get());
    REQUIRE(grouping != nullptr);
  }

  SECTION("Grouping with unary operator") {
    auto stmnt = helpers::getStmnt("-(3+4)");
    auto* unary = helpers::getTopExpr<Unary>(stmnt);
    REQUIRE(unary->m_op.m_type == TokenType::MINUS);
    auto* grouping = dynamic_cast<Grouping*>(unary->m_right.get());
    REQUIRE(grouping != nullptr);
  }

  SECTION("String grouping expression") {
    auto stmnt = helpers::getStmnt(R"(("hello"+"world"))");
    auto* grouping = helpers::getTopExpr<Grouping>(stmnt);
    auto* binary = dynamic_cast<Binary*>(grouping->m_expr.get());
    REQUIRE(binary != nullptr);
    REQUIRE(binary->m_op.m_type == TokenType::PLUS);
  }
}

TEST_CASE("Test unary expression parsing", "[parser][expression][unary]") {
  SECTION("Unary expression on number") {
    auto stmnt = helpers::getStmnt("-5");
    auto* unary = helpers::getTopExpr<Unary>(stmnt);
    REQUIRE(unary->m_op.m_type == TokenType::MINUS);
  }

  SECTION("Unary expression on boolean") {
    auto stmnt = helpers::getStmnt("!true");
    auto* unary = helpers::getTopExpr<Unary>(stmnt);
    REQUIRE(unary->m_op.m_type == TokenType::BANG);
  }
}

TEST_CASE("Test arithmetic operator parsing",
          "[parser][expression][operator][binary]") {
  auto [op, tokenType] =
      GENERATE(table<std::string, TokenType>({{"+", TokenType::PLUS},
                                              {"-", TokenType::MINUS},
                                              {"*", TokenType::STAR},
                                              {"/", TokenType::SLASH}}));

  auto stmnt = helpers::getStmnt(std::format("3{}5", op));
  auto* binary = helpers::getTopExpr<Binary>(stmnt);
  REQUIRE(binary->m_op.m_type == tokenType);
}

TEST_CASE("Test arithmetic operator precedence and associativity",
          "[parser][expression][operator][binary]") {
  SECTION("Multiplication before addition") {
    auto stmnt = helpers::getStmnt("2+3*4");
    auto* binary = helpers::getTopExpr<Binary>(stmnt);
    REQUIRE(binary->m_op.m_type == TokenType::PLUS);

    auto* rhs = dynamic_cast<Binary*>(binary->m_right.get());
    REQUIRE(rhs != nullptr);
    REQUIRE(rhs->m_op.m_type == TokenType::STAR);
  }

  SECTION("Division before subtraction") {
    auto stmnt = helpers::getStmnt("3-4/2");
    auto* binary = helpers::getTopExpr<Binary>(stmnt);
    REQUIRE(binary->m_op.m_type == TokenType::MINUS);

    auto* rhs = dynamic_cast<Binary*>(binary->m_right.get());
    REQUIRE(rhs != nullptr);
    REQUIRE(rhs->m_op.m_type == TokenType::SLASH);
  }

  SECTION("Associativity") {
    auto stmnt = helpers::getStmnt("5+3-2");
    auto* binary = helpers::getTopExpr<Binary>(stmnt);
    REQUIRE(binary->m_op.m_type == TokenType::MINUS);

    auto* lhs = dynamic_cast<Binary*>(binary->m_left.get());
    REQUIRE(lhs != nullptr);
    REQUIRE(lhs->m_op.m_type == TokenType::PLUS);
  }
}

TEST_CASE("Test comparison operators", "[parser][expressions][operators]") {
  SECTION("Basic operator parsing") {
    auto [op, tokenType] = GENERATE(
        table<std::string, TokenType>({{"<", TokenType::LESS},
                                       {">", TokenType::GREATER},
                                       {"<=", TokenType::LESS_EQUAL},
                                       {">=", TokenType::GREATER_EQUAL}}));

    auto stmnt = helpers::getStmnt(std::format("3{}5", op));
    auto* binary = helpers::getTopExpr<Binary>(stmnt);
    REQUIRE(binary->m_op.m_type == tokenType);
  }

  SECTION("Arithmetic before comparison") {
    auto stmnt = helpers::getStmnt("2 + 3 < 4 * 2");
    auto* binary = helpers::getTopExpr<Binary>(stmnt);
    REQUIRE(binary->m_op.m_type == TokenType::LESS);

    auto* lhs = dynamic_cast<Binary*>(binary->m_left.get());
    REQUIRE(lhs != nullptr);
    REQUIRE(lhs->m_op.m_type == TokenType::PLUS);

    auto* rhs = dynamic_cast<Binary*>(binary->m_right.get());
    REQUIRE(rhs != nullptr);
    REQUIRE(rhs->m_op.m_type == TokenType::STAR);
  }

  SECTION("Subtraction before comparison") {
    auto stmnt = helpers::getStmnt("10 - 2 > 5");
    auto* binary = helpers::getTopExpr<Binary>(stmnt);
    REQUIRE(binary->m_op.m_type == TokenType::GREATER);

    auto* lhs = dynamic_cast<Binary*>(binary->m_left.get());
    REQUIRE(lhs != nullptr);
    REQUIRE(lhs->m_op.m_type == TokenType::MINUS);
  }

  SECTION("Comparison before equality") {
    auto stmnt = helpers::getStmnt("3+5 == 5+3");
    auto* binary = helpers::getTopExpr<Binary>(stmnt);
    REQUIRE(binary->m_op.m_type == TokenType::EQUAL_EQUAL);

    auto* lhs = dynamic_cast<Binary*>(binary->m_left.get());
    REQUIRE(lhs != nullptr);
    REQUIRE(lhs->m_op.m_type == TokenType::PLUS);

    auto* rhs = dynamic_cast<Binary*>(binary->m_right.get());
    REQUIRE(rhs != nullptr);
    REQUIRE(rhs->m_op.m_type == TokenType::PLUS);
  }
}

TEST_CASE("Test equality operator parsing",
          "[parser][expressions][operators]") {
  auto [op, tokenType] = GENERATE(table<std::string, TokenType>(
      {{"==", TokenType::EQUAL_EQUAL}, {"!=", TokenType::BANG_EQUAL}}));

  auto stmnt = helpers::getStmnt(std::format("3{}5", op));
  auto* binary = helpers::getTopExpr<Binary>(stmnt);
  REQUIRE(binary->m_op.m_type == tokenType);
}

TEST_CASE("Test parser error handling", "[parser][expressions][errors]") {
  SECTION("Invalid primary expression") {
    auto captured = helpers::captureStream(std::cerr, helpers::getStmnt, ")");
    REQUIRE(captured.find("Expected expression") != std::string::npos);
  }

  SECTION("Missing closing parenthesis") {
    auto captured = helpers::captureStream(std::cerr, helpers::getStmnt, "(5");
    REQUIRE(captured.find("Expect \')\' after expression") !=
            std::string::npos);
  }

  SECTION("Variable declaration without name") {
    auto captured = helpers::captureStream(std::cerr, helpers::getStmnt, "var");
    REQUIRE(captured.find("Expect variable name") != std::string::npos);
  }

  SECTION("Invalid assignment target") {
    auto captured =
        helpers::captureStream(std::cerr, helpers::getStmnt, "5=10");
    REQUIRE(captured.find("Invalid assignment target") != std::string::npos);
  }
}

TEST_CASE("Test expression str() methods", "[expr][str]") {
  SECTION("Literal str()") {
    Literal numLit(42.0);
    REQUIRE(numLit.str() == "42");

    Literal strLit(std::string("hello"));
    REQUIRE(strLit.str() == "hello");

    Literal boolLit(true);
    REQUIRE(boolLit.str() == "true");

    Literal nilLit(std::monostate{});
    REQUIRE(nilLit.str() == "(null)");
  }

  SECTION("Variable str()") {
    Token tok(TokenType::IDENTIFIER, "myvar", std::monostate{});
    Variable var(tok);
    REQUIRE(var.str().find("myvar") != std::string::npos);
  }

  SECTION("Unary str()") {
    Token minusOp(TokenType::MINUS, "-", std::monostate{});
    auto lit = std::make_unique<Literal>(5.0);
    Unary unary(minusOp, std::move(lit));
    auto str = unary.str();
    REQUIRE(str.find('-') != std::string::npos);
    REQUIRE(str.find('5') != std::string::npos);
  }

  SECTION("Binary str()") {
    Token plusOp(TokenType::PLUS, "+", std::monostate{});
    auto left = std::make_unique<Literal>(2.0);
    auto right = std::make_unique<Literal>(3.0);
    Binary binary(std::move(left), plusOp, std::move(right));
    auto str = binary.str();
    REQUIRE(str.find('+') != std::string::npos);
    REQUIRE(str.find('2') != std::string::npos);
    REQUIRE(str.find('3') != std::string::npos);
  }

  SECTION("Grouping str()") {
    auto lit = std::make_unique<Literal>(42.0);
    Grouping group(std::move(lit));
    REQUIRE(group.str().find("42") != std::string::npos);
  }

  SECTION("Assign str()") {
    Token name(TokenType::IDENTIFIER, "x", std::monostate{});
    auto value = std::make_unique<Literal>(10.0);
    Assign assign(name, std::move(value));
    auto str = assign.str();
    REQUIRE(str.find('x') != std::string::npos);
    REQUIRE(str.find("10") != std::string::npos);
  }
}

TEST_CASE("Test expression formatters", "[expr][formatter]") {
  SECTION("Format unique_ptr<Expr>") {
    std::unique_ptr<Expr> expr = std::make_unique<Literal>(123.0);
    std::string formatted = std::format("{}", expr);
    REQUIRE(formatted == "123");
  }

  SECTION("Format null unique_ptr<Expr>") {
    std::unique_ptr<Expr> expr = nullptr;
    std::string formatted = std::format("{}", expr);
    REQUIRE(formatted == "null");
  }

  SECTION("Format Expr*") {
    Literal lit(456.0);
    Expr* ptr = &lit;
    std::string formatted = std::format("{}", ptr);
    REQUIRE(formatted == "456");
  }

  SECTION("Format null Expr*") {
    Expr* ptr = nullptr;
    std::string formatted = std::format("{}", ptr);
    REQUIRE(formatted == "null");
  }
}
