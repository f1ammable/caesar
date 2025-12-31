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

  CAPTURE(lhs, rhs);

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
