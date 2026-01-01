#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <variant>

#include "object.hpp"
#include "test_helpers.hpp"

TEST_CASE("Test binary operation", "[object][operator]") {
  auto lhs = GENERATE(Object{5.0}, Object{true}, Object{"hello"},
                      Object{std::monostate{}});

  auto rhs = GENERATE(Object{3.0}, Object{false}, Object{" world"},
                      Object{std::monostate{}});

  bool lhs_is_num =
      std::holds_alternative<double>(lhs) || std::holds_alternative<bool>(lhs);
  bool rhs_is_num =
      std::holds_alternative<double>(rhs) || std::holds_alternative<bool>(rhs);
  bool lhs_is_str = std::holds_alternative<std::string>(lhs);
  bool rhs_is_str = std::holds_alternative<std::string>(rhs);

  if (lhs_is_num && rhs_is_num) {
    Object res = lhs + rhs;
    REQUIRE(std::holds_alternative<double>(res));
  } else if (lhs_is_str && rhs_is_str) {
    Object res = lhs + rhs;
    REQUIRE(std::holds_alternative<std::string>(res));
  } else {
    auto captured = helpers::captureStream(
        std::cerr, [](const Object& l, const Object& r) { return l + r; }, lhs,
        rhs);
    REQUIRE(captured.find("Unsupported operand types") != std::string::npos);
  }
}

TEST_CASE("Test operator behaviour for string operations",
          "[object][operator]") {
  // Omit + operator as its covered by test above
  auto [op_name, op] = GENERATE(
      table<std::string, std::function<Object(const Object&, const Object&)>>(
          {{"-",
            [](const Object& lhs, const Object& rhs) { return lhs - rhs; }},
           {"*",
            [](const Object& lhs, const Object& rhs) { return lhs * rhs; }},
           {"/",
            [](const Object& lhs, const Object& rhs) { return lhs / rhs; }}}));

  Object lhs = Object{"hello"};
  Object rhs = Object{"rhs"};
  auto captured = helpers::captureStream(std::cerr, op, lhs, rhs);
  REQUIRE(captured.find(std::format("Cannot apply operator {} to strings",
                                    op_name)) != std::string::npos);
}

TEST_CASE("Test comparison operators", "[object][operator]") {
  auto [op_name, op] = GENERATE(
      table<std::string, std::function<Object(const Object&, const Object&)>>(
          {{"<",
            [](const Object& lhs, const Object& rhs) { return lhs < rhs; }},
           {">",
            [](const Object& lhs, const Object& rhs) { return lhs > rhs; }},
           {"<=",
            [](const Object& lhs, const Object& rhs) { return lhs <= rhs; }},
           {">=",
            [](const Object& lhs, const Object& rhs) { return lhs >= rhs; }}}));

  auto lhs = GENERATE(Object{5.0}, Object{true}, Object{"hello"},
                      Object{std::monostate{}});

  auto rhs = GENERATE(Object{3.0}, Object{false}, Object{" world"},
                      Object{std::monostate{}});

  bool lhs_is_num =
      std::holds_alternative<double>(lhs) || std::holds_alternative<bool>(lhs);
  bool rhs_is_num =
      std::holds_alternative<double>(rhs) || std::holds_alternative<bool>(rhs);

  if (lhs_is_num && rhs_is_num) {
    Object res = lhs + rhs;
    REQUIRE(std::holds_alternative<double>(res));
  } else {
    auto captured = helpers::captureStream(std::cerr, op, lhs, rhs);
    REQUIRE(captured.find(std::format(
                "Cannot apply operator {} to non-arithmetic types", op_name)) !=
            std::string::npos);
  }
}

TEST_CASE("Test equality operators", "[object][operator]") {
  auto lhs = GENERATE(Object{5.0}, Object{true}, Object{"hello"},
                      Object{std::monostate{}});

  auto rhs = GENERATE(Object{5.0}, Object{true}, Object{"hello"},
                      Object{std::monostate{}});

  auto [op_name, op,
        res] = GENERATE(table<std::string,
                              std::function<bool(const Object&, const Object&)>,
                              bool>(
      {{"==", [](const Object& lhs, const Object& rhs) { return lhs == rhs; },
        true},
       {"!=", [](const Object& lhs, const Object& rhs) { return lhs != rhs; },
        false}}));

  if (lhs.index() == rhs.index())
    REQUIRE(op(lhs, rhs) == res);
  else
    REQUIRE(op(lhs, rhs) == !res);
}
