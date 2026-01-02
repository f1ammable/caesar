#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <variant>

#include "object.hpp"
#include "test_helpers.hpp"

// This whole file is ugly but it works and I can't find it a better way to do
// it

TEST_CASE("Test arithmetic operators between all type combinations",
          "[object][operator][arithmetic]") {
  auto [op_name, op, lhs, rhs, should_succeed, expected_type] = GENERATE(
      table<std::string, std::function<Object(const Object&, const Object&)>,
            Object, Object, bool, std::string>({
          // Addition
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{5.0}, Object{3.0}, true, "double"},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{5.0}, Object{true}, true, "double"},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{true}, Object{3.0}, true, "double"},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{true}, Object{false}, true, "double"},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{"hello"}, Object{" world"}, true, "string"},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{5.0}, Object{"hello"}, false, ""},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{"hello"}, Object{5.0}, false, ""},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{true}, Object{"hello"}, false, ""},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{"hello"}, Object{true}, false, ""},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{std::monostate{}}, Object{5.0}, false, ""},
          {"+", [](const Object& l, const Object& r) { return l + r; },
           Object{5.0}, Object{std::monostate{}}, false, ""},

          // Subtraction
          {"-", [](const Object& l, const Object& r) { return l - r; },
           Object{5.0}, Object{3.0}, true, "double"},
          {"-", [](const Object& l, const Object& r) { return l - r; },
           Object{5.0}, Object{true}, true, "double"},
          {"-", [](const Object& l, const Object& r) { return l - r; },
           Object{true}, Object{false}, true, "double"},
          {"-", [](const Object& l, const Object& r) { return l - r; },
           Object{"hello"}, Object{"world"}, false, ""},
          {"-", [](const Object& l, const Object& r) { return l - r; },
           Object{5.0}, Object{"hello"}, false, ""},
          {"-", [](const Object& l, const Object& r) { return l - r; },
           Object{std::monostate{}}, Object{5.0}, false, ""},

          // Multiplication
          {"*", [](const Object& l, const Object& r) { return l * r; },
           Object{5.0}, Object{3.0}, true, "double"},
          {"*", [](const Object& l, const Object& r) { return l * r; },
           Object{5.0}, Object{true}, true, "double"},
          {"*", [](const Object& l, const Object& r) { return l * r; },
           Object{true}, Object{false}, true, "double"},
          {"*", [](const Object& l, const Object& r) { return l * r; },
           Object{"hello"}, Object{"world"}, false, ""},
          {"*", [](const Object& l, const Object& r) { return l * r; },
           Object{5.0}, Object{"hello"}, false, ""},
          {"*", [](const Object& l, const Object& r) { return l * r; },
           Object{std::monostate{}}, Object{5.0}, false, ""},

          // Division
          {"/", [](const Object& l, const Object& r) { return l / r; },
           Object{6.0}, Object{3.0}, true, "double"},
          {"/", [](const Object& l, const Object& r) { return l / r; },
           Object{5.0}, Object{true}, true, "double"},
          {"/", [](const Object& l, const Object& r) { return l / r; },
           Object{true}, Object{true}, true, "double"},
          {"/", [](const Object& l, const Object& r) { return l / r; },
           Object{"hello"}, Object{"world"}, false, ""},
          {"/", [](const Object& l, const Object& r) { return l / r; },
           Object{5.0}, Object{"hello"}, false, ""},
          {"/", [](const Object& l, const Object& r) { return l / r; },
           Object{std::monostate{}}, Object{5.0}, false, ""},
      }));

  if (should_succeed) {
    Object res = op(lhs, rhs);
    if (expected_type == "double") {
      REQUIRE(std::holds_alternative<double>(res));
    } else if (expected_type == "string") {
      REQUIRE(std::holds_alternative<std::string>(res));
    }
  } else {
    auto captured = helpers::captureStream(std::cerr, op, lhs, rhs);
    bool has_string_error =
        captured.find(std::format("Cannot apply operator {} to strings",
                                  op_name)) != std::string::npos;
    bool has_unsupported_error =
        captured.find("Unsupported operand types") != std::string::npos;
    REQUIRE((has_string_error || has_unsupported_error));
  }
}

TEST_CASE("Test comparison operators between all type combinations",
          "[object][operator][comparison]") {
  auto [op_name, op, lhs, rhs, should_succeed] = GENERATE(
      table<std::string, std::function<bool(const Object&, const Object&)>,
            Object, Object, bool>({
          // Less than
          {"<", [](const Object& l, const Object& r) { return l < r; },
           Object{5.0}, Object{3.0}, true},
          {"<", [](const Object& l, const Object& r) { return l < r; },
           Object{5.0}, Object{true}, true},
          {"<", [](const Object& l, const Object& r) { return l < r; },
           Object{true}, Object{false}, true},
          {"<", [](const Object& l, const Object& r) { return l < r; },
           Object{"hello"}, Object{"world"}, false},
          {"<", [](const Object& l, const Object& r) { return l < r; },
           Object{5.0}, Object{"hello"}, false},
          {"<", [](const Object& l, const Object& r) { return l < r; },
           Object{std::monostate{}}, Object{5.0}, false},

          // Greater than
          {">", [](const Object& l, const Object& r) { return l > r; },
           Object{5.0}, Object{3.0}, true},
          {">", [](const Object& l, const Object& r) { return l > r; },
           Object{5.0}, Object{true}, true},
          {">", [](const Object& l, const Object& r) { return l > r; },
           Object{true}, Object{false}, true},
          {">", [](const Object& l, const Object& r) { return l > r; },
           Object{"hello"}, Object{"world"}, false},
          {">", [](const Object& l, const Object& r) { return l > r; },
           Object{5.0}, Object{"hello"}, false},
          {">", [](const Object& l, const Object& r) { return l > r; },
           Object{std::monostate{}}, Object{5.0}, false},

          // Less than or equal to
          {"<=", [](const Object& l, const Object& r) { return l <= r; },
           Object{5.0}, Object{3.0}, true},
          {"<=", [](const Object& l, const Object& r) { return l <= r; },
           Object{5.0}, Object{true}, true},
          {"<=", [](const Object& l, const Object& r) { return l <= r; },
           Object{true}, Object{false}, true},
          {"<=", [](const Object& l, const Object& r) { return l <= r; },
           Object{"hello"}, Object{"world"}, false},
          {"<=", [](const Object& l, const Object& r) { return l <= r; },
           Object{5.0}, Object{"hello"}, false},
          {"<=", [](const Object& l, const Object& r) { return l <= r; },
           Object{std::monostate{}}, Object{5.0}, false},

          // Greater than or equal to
          {">=", [](const Object& l, const Object& r) { return l >= r; },
           Object{5.0}, Object{3.0}, true},
          {">=", [](const Object& l, const Object& r) { return l >= r; },
           Object{5.0}, Object{true}, true},
          {">=", [](const Object& l, const Object& r) { return l >= r; },
           Object{true}, Object{false}, true},
          {">=", [](const Object& l, const Object& r) { return l >= r; },
           Object{"hello"}, Object{"world"}, false},
          {">=", [](const Object& l, const Object& r) { return l >= r; },
           Object{5.0}, Object{"hello"}, false},
          {">=", [](const Object& l, const Object& r) { return l >= r; },
           Object{std::monostate{}}, Object{5.0}, false},
      }));

  if (should_succeed) {
    bool result = op(lhs, rhs);
    REQUIRE((result == true || result == false));
  } else {
    auto captured = helpers::captureStream(std::cerr, op, lhs, rhs);
    REQUIRE(captured.find(std::format(
                "Cannot apply operator {} to non-arithmetic types", op_name)) !=
            std::string::npos);
  }
}

TEST_CASE("Test equality operators between all type combinations",
          "[object][operator][equality]") {
  auto [op_name, op, lhs, rhs, expected_result] = GENERATE(
      table<std::string, std::function<bool(const Object&, const Object&)>,
            Object, Object, bool>({
          // == operator - same types
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{5.0}, Object{5.0}, true},
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{true}, Object{true}, true},
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{"hello"}, Object{"hello"}, true},
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{std::monostate{}}, Object{std::monostate{}}, true},

          // == operator - different values, same types
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{5.0}, Object{3.0}, false},
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{true}, Object{false}, false},
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{"hello"}, Object{"world"}, false},

          // == operator - different types
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{5.0}, Object{true}, false},
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{5.0}, Object{"hello"}, false},
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{true}, Object{"hello"}, false},
          {"==", [](const Object& l, const Object& r) { return l == r; },
           Object{std::monostate{}}, Object{5.0}, false},

          // != operator - same types
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{5.0}, Object{5.0}, false},
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{true}, Object{true}, false},
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{"hello"}, Object{"hello"}, false},
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{std::monostate{}}, Object{std::monostate{}}, false},

          // != operator - different values, same types
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{5.0}, Object{3.0}, true},
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{true}, Object{false}, true},
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{"hello"}, Object{"world"}, true},

          // != operator - different types
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{5.0}, Object{true}, true},
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{5.0}, Object{"hello"}, true},
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{true}, Object{"hello"}, true},
          {"!=", [](const Object& l, const Object& r) { return l != r; },
           Object{std::monostate{}}, Object{5.0}, true},
      }));

  bool result = op(lhs, rhs);
  REQUIRE(result == expected_result);
}
