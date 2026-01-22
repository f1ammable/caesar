#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cmd/interpreter.hpp>
#include <cmd/object.hpp>
#include <memory>
#include <variant>

#include "test_helpers.hpp"

TEST_CASE("Test truthy behavior through bang operator",
          "[interpreter][truthy]") {
  Interpreter interp;

  SECTION("False and nil are falsy") {
    auto stmnt1 = helpers::getStmnt("!!false");
    Object result1 = interp.interpret(stmnt1);
    REQUIRE(result1 == Object{false});

    auto stmnt2 = helpers::getStmnt("!!nil");
    Object result2 = interp.interpret(stmnt2);
    REQUIRE(result2 == Object{false});
  }

  SECTION("Everything else is truthy") {
    auto [input] = GENERATE(table<std::string>({
        {"!!true"},
        {"!!0"},
        {"!!1"},
        {"!!-1"},
        {"!!\"\""},
        {"!!\"hello\""},
    }));

    auto stmnt = helpers::getStmnt(input);
    Object result = interp.interpret(stmnt);
    REQUIRE(result == Object{true});
  }

  SECTION("Single bang inverts truthiness") {
    auto stmnt1 = helpers::getStmnt("!false");
    REQUIRE(interp.interpret(stmnt1) == Object{true});

    auto stmnt2 = helpers::getStmnt("!true");
    REQUIRE(interp.interpret(stmnt2) == Object{false});

    auto stmnt3 = helpers::getStmnt("!nil");
    REQUIRE(interp.interpret(stmnt3) == Object{true});

    auto stmnt4 = helpers::getStmnt("!5");
    REQUIRE(interp.interpret(stmnt4) == Object{false});
  }
}

TEST_CASE("Test stringify with different object types",
          "[interpreter][stringify]") {
  Interpreter interp;

  SECTION("Numbers") {
    REQUIRE(interp.stringify(Object{42.0}) == "42");
    REQUIRE(interp.stringify(Object{3.14}) == "3.14");
    REQUIRE(interp.stringify(Object{0.0}) == "0");
    REQUIRE(interp.stringify(Object{-5.5}) == "-5.5");
  }

  SECTION("Booleans") {
    REQUIRE(interp.stringify(Object{true}) == "true");
    REQUIRE(interp.stringify(Object{false}) == "false");
  }

  SECTION("Strings") {
    REQUIRE(interp.stringify(Object{"hello"}) == "hello");
    REQUIRE(interp.stringify(Object{""}) == "");
    REQUIRE(interp.stringify(Object{"test string"}) == "test string");
  }

  SECTION("Nil/monostate") {
    REQUIRE(interp.stringify(Object{std::monostate{}}) == "(null)");
  }
}

TEST_CASE("Test literal expression evaluation", "[interpreter][evaluate]") {
  Interpreter interp;

  auto [input, expected_value] = GENERATE(table<std::string, Object>({
      {"42", Object{42.0}},
      {"3.14", Object{3.14}},
      {"true", Object{true}},
      {"false", Object{false}},
      {"\"hello\"", Object{"hello"}},
      {"nil", Object{std::monostate{}}},
  }));

  auto stmnt = helpers::getStmnt(input);
  Object result = interp.interpret(stmnt);

  REQUIRE(result == expected_value);
}

TEST_CASE("Test grouping expression evaluation", "[interpreter][evaluate]") {
  Interpreter interp;

  SECTION("Simple grouping") {
    auto stmnt = helpers::getStmnt("(5)");
    Object result = interp.interpret(stmnt);
    REQUIRE(result == Object{5.0});
  }

  SECTION("Grouping with expression") {
    auto stmnt = helpers::getStmnt("(2 + 3)");
    Object result = interp.interpret(stmnt);
    REQUIRE(result == Object{5.0});
  }

  SECTION("Nested grouping") {
    auto stmnt = helpers::getStmnt("((5))");
    Object result = interp.interpret(stmnt);
    REQUIRE(result == Object{5.0});
  }
}

TEST_CASE("Test unary expression evaluation",
          "[interpreter][evaluate][unary]") {
  Interpreter interp;

  SECTION("Unary minus on numbers") {
    auto stmnt = helpers::getStmnt("-5");
    Object result = interp.interpret(stmnt);
    REQUIRE(result == Object{-5.0});

    stmnt = helpers::getStmnt("-(-3)");
    result = interp.interpret(stmnt);
    REQUIRE(result == Object{3.0});
  }

  SECTION("Unary bang on booleans") {
    auto stmnt = helpers::getStmnt("!true");
    Object result = interp.interpret(stmnt);
    REQUIRE(result == Object{false});

    stmnt = helpers::getStmnt("!false");
    result = interp.interpret(stmnt);
    REQUIRE(result == Object{true});
  }

  SECTION("Unary bang on other types") {
    auto stmnt = helpers::getStmnt("!5");
    Object result = interp.interpret(stmnt);
    REQUIRE(result == Object{false});

    stmnt = helpers::getStmnt("!nil");
    result = interp.interpret(stmnt);
    REQUIRE(result == Object{true});

    stmnt = helpers::getStmnt("!\"hello\"");
    result = interp.interpret(stmnt);
    REQUIRE(result == Object{false});
  }
}

TEST_CASE("Test binary arithmetic expression evaluation",
          "[interpreter][evaluate][binary]") {
  Interpreter interp;

  auto [input, expected] = GENERATE(table<std::string, double>({
      {"2 + 3", 5.0},
      {"10 - 4", 6.0},
      {"3 * 4", 12.0},
      {"15 / 3", 5.0},
      {"2 + 3 * 4", 14.0},
      {"(2 + 3) * 4", 20.0},
      {"10 - 2 - 3", 5.0},
  }));

  auto stmnt = helpers::getStmnt(input);
  Object result = interp.interpret(stmnt);
  REQUIRE(std::get<double>(result) == expected);
}

TEST_CASE("Test binary comparison expression evaluation",
          "[interpreter][evaluate][binary]") {
  Interpreter interp;

  auto [input, expected] = GENERATE(table<std::string, bool>({
      {"5 > 3", true},
      {"3 > 5", false},
      {"5 >= 5", true},
      {"3 >= 5", false},
      {"3 < 5", true},
      {"5 < 3", false},
      {"5 <= 5", true},
      {"5 <= 3", false},
  }));

  auto stmnt = helpers::getStmnt(input);
  Object result = interp.interpret(stmnt);
  REQUIRE(std::get<bool>(result) == expected);
}

TEST_CASE("Test binary equality expression evaluation",
          "[interpreter][evaluate][binary]") {
  Interpreter interp;

  auto [input, expected] = GENERATE(table<std::string, bool>({
      {"5 == 5", true},
      {"5 == 3", false},
      {"5 != 3", true},
      {"5 != 5", false},
      {"true == true", true},
      {"true == false", false},
      {"\"hello\" == \"hello\"", true},
      {"\"hello\" == \"world\"", false},
      {"nil == nil", true},
      {"5 == true", false},
      {"5 != true", true},
  }));

  auto stmnt = helpers::getStmnt(input);
  Object result = interp.interpret(stmnt);
  REQUIRE(std::get<bool>(result) == expected);
}

TEST_CASE("Test visitVarStmnt - declaration with initializer",
          "[interpreter][visitVarStmnt]") {
  Interpreter interp;
  auto stmnt = helpers::getStmnt("var testvar1 = 42");
  Object result = interp.interpret(stmnt);
  REQUIRE(result == Object{std::monostate{}});
}

TEST_CASE("Test visitVarStmnt - declaration without initializer",
          "[interpreter][visitVarStmnt]") {
  Interpreter interp;
  auto stmnt = helpers::getStmnt("var testvar2");
  Object result = interp.interpret(stmnt);
  REQUIRE(result == Object{std::monostate{}});
}

TEST_CASE("Test visitVariableExpr - retrieve number variable",
          "[interpreter][visitVariableExpr]") {
  Interpreter interp;
  auto declStmnt = helpers::getStmnt("var numvar = 123");
  interp.interpret(declStmnt);

  auto varStmnt = helpers::getStmnt("numvar");
  Object result = interp.interpret(varStmnt);
  REQUIRE(result == Object{123.0});
}

TEST_CASE("Test visitVariableExpr - retrieve string variable",
          "[interpreter][visitVariableExpr]") {
  Interpreter interp;
  auto declStmnt = helpers::getStmnt("var stringvar = \"test\"");
  interp.interpret(declStmnt);

  auto varStmnt = helpers::getStmnt("stringvar");
  Object result = interp.interpret(varStmnt);
  REQUIRE(result == Object{"test"});
}

TEST_CASE("Test visitVariableExpr - retrieve boolean variable",
          "[interpreter][visitVariableExpr]") {
  Interpreter interp;
  auto declStmnt = helpers::getStmnt("var boolvar = true");
  interp.interpret(declStmnt);

  auto varStmnt = helpers::getStmnt("boolvar");
  Object result = interp.interpret(varStmnt);
  REQUIRE(result == Object{true});
}

TEST_CASE("Test visitAssignExpr - assignment returns value",
          "[interpreter][visitAssignExpr]") {
  Interpreter interp;
  auto declStmnt = helpers::getStmnt("var assignvar = 10");
  interp.interpret(declStmnt);

  auto assignStmnt = helpers::getStmnt("assignvar = 20");
  Object result = interp.interpret(assignStmnt);
  REQUIRE(result == Object{20.0});
}

TEST_CASE("Test visitAssignExpr - assignment with expression",
          "[interpreter][visitAssignExpr]") {
  Interpreter interp;
  auto declStmnt = helpers::getStmnt("var exprvar = 5");
  interp.interpret(declStmnt);

  auto assignStmnt = helpers::getStmnt("exprvar = exprvar + 10");
  Object result = interp.interpret(assignStmnt);
  REQUIRE(result == Object{15.0});
}

TEST_CASE("Test visitAssignExpr - assignment changes value",
          "[interpreter][visitAssignExpr]") {
  Interpreter interp;
  auto declStmnt = helpers::getStmnt("var changevar = 1");
  interp.interpret(declStmnt);

  auto assignStmnt = helpers::getStmnt("changevar = 100");
  interp.interpret(assignStmnt);

  auto varStmnt = helpers::getStmnt("changevar");
  Object result = interp.interpret(varStmnt);
  REQUIRE(result == Object{100.0});
}

TEST_CASE("Test visitCallStmnt - print function",
          "[interpreter][visitCallStmnt]") {
  Interpreter interp;

  auto stmnt = helpers::getStmnt("print 42");
  REQUIRE_NOTHROW(interp.interpret(stmnt));
}

TEST_CASE("Test visitCallStmnt - function argument mismatch errors",
          "[interpreter][visitCallStmnt][error]") {
  Interpreter interp;

  SECTION("Too many arguments to print") {
    auto captured = helpers::captureStream(std::cerr, [&interp]() {
      auto stmnt = helpers::getStmnt("print 42 43");
      interp.interpret(stmnt);
    });
    REQUIRE(
        captured.find("Function requires 1 arguments but 2 were provided") !=
        std::string::npos);
  }
}

TEST_CASE("Test variable in arithmetic expression",
          "[interpreter][integration]") {
  Interpreter interp;
  auto declStmnt = helpers::getStmnt("var mathvartest = 5");
  interp.interpret(declStmnt);

  auto exprStmnt = helpers::getStmnt("mathvartest * 2 + 3");
  Object result = interp.interpret(exprStmnt);
  REQUIRE(result == Object{13.0});
}

TEST_CASE("Test multiple variables in expression",
          "[interpreter][integration]") {
  Interpreter interp;
  auto decl1 = helpers::getStmnt("var avartest = 10");
  interp.interpret(decl1);

  auto decl2 = helpers::getStmnt("var bvartest = 5");
  interp.interpret(decl2);

  auto exprStmnt = helpers::getStmnt("avartest - bvartest");
  Object result = interp.interpret(exprStmnt);
  REQUIRE(result == Object{5.0});
}
