#include <catch2/catch_test_macros.hpp>
#include <cmd/stdlib.hpp>
#include <variant>

#include "test_helpers.hpp"

TEST_CASE("Test PrintFn", "[stdlib][print]") {
  PrintFn print;

  SECTION("arity and str") {
    REQUIRE(print.arity() == 1);
    REQUIRE(print.str() == "<native fn: print>");
  }

  SECTION("call returns first argument") {
    std::vector<Object> args = {42.0};
    Object result = print.call(args);
    REQUIRE(std::get<double>(result) == 42.0);
  }

  SECTION("call with string") {
    std::vector<Object> args = {std::string("test")};
    Object result = print.call(args);
    REQUIRE(std::get<std::string>(result) == "test");
  }
}

TEST_CASE("Test BreakpointFn without target", "[stdlib][breakpoint]") {
  BreakpointFn bp;

  SECTION("arity and str") {
    REQUIRE(bp.arity() == 1);
    REQUIRE(bp.str() == "<native fn: breakpoint>");
  }

  SECTION("call when target is null produces error") {
    std::vector<Object> args = {std::string("list")};
    auto captured = helpers::captureStream(std::cerr, [&bp, &args]() {
      Object result = bp.call(args);
      REQUIRE(std::holds_alternative<std::monostate>(result));
    });
    REQUIRE(captured.find("Target is not running") != std::string::npos);
  }
}

TEST_CASE("Test RunFn without target", "[stdlib][run]") {
  RunFn run;

  SECTION("arity and str") {
    REQUIRE(run.arity() == 0);
    REQUIRE(run.str() == "<native fn: run>");
  }

  SECTION("call when target is null produces error") {
    std::vector<Object> args = {};
    auto captured = helpers::captureStream(std::cerr, [&run, &args]() {
      Object result = run.call(args);
      REQUIRE(std::holds_alternative<std::monostate>(result));
    });
    REQUIRE(captured.find("Target is not set") != std::string::npos);
  }
}

TEST_CASE("Test ContinueFn properties", "[stdlib][continue]") {
  ContinueFn cont;

  SECTION("arity and str") {
    REQUIRE(cont.arity() == 0);
    REQUIRE(cont.str() == "<native fn: continue>");
  }
}
