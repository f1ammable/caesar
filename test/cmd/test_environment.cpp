#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cmd/environment.hpp>
#include <cmd/object.hpp>
#include <cmd/subcommand.hpp>
#include <format>
#include <variant>

#include "test_helpers.hpp"

TEST_CASE("Test environment singleton pattern", "[environment][singleton]") {
  Environment& env1 = Environment::getInstance();
  Environment& env2 = Environment::getInstance();
  REQUIRE(&env1 == &env2);
}

TEST_CASE("Test environment define with different types",
          "[environment][define]") {
  Environment& env = Environment::getInstance();

  auto [var_name, value, type_name] =
      GENERATE(table<std::string, Object, std::string>({
          {"test_var_double", Object{42.0}, "double"},
          {"test_var_bool", Object{true}, "bool"},
          {"test_var_string", Object{"hello"}, "string"},
          {"test_var_monostate", Object{std::monostate{}}, "monostate"},
      }));

  DYNAMIC_SECTION(std::format("Define and get {} variable", type_name)) {
    env.define(var_name, value);
    Object retrieved = env.get(helpers::makeToken(var_name));

    if (type_name == "double") {
      REQUIRE(std::holds_alternative<double>(retrieved));
      REQUIRE(std::get<double>(retrieved) == std::get<double>(value));
    } else if (type_name == "bool") {
      REQUIRE(std::holds_alternative<bool>(retrieved));
      REQUIRE(std::get<bool>(retrieved) == std::get<bool>(value));
    } else if (type_name == "string") {
      REQUIRE(std::holds_alternative<std::string>(retrieved));
      REQUIRE(std::get<std::string>(retrieved) == std::get<std::string>(value));
    } else if (type_name == "monostate") {
      REQUIRE(std::holds_alternative<std::monostate>(retrieved));
    }
  }
}

TEST_CASE("Test environment define can update existing variable",
          "[environment][define]") {
  Environment& env = Environment::getInstance();

  env.define("test_update_var", Object{10.0});
  Object first = env.get(helpers::makeToken("test_update_var"));
  REQUIRE(std::holds_alternative<double>(first));
  REQUIRE(std::get<double>(first) == 10.0);

  env.define("test_update_var", Object{20.0});
  Object second = env.get(helpers::makeToken("test_update_var"));
  REQUIRE(std::holds_alternative<double>(second));
  REQUIRE(std::get<double>(second) == 20.0);
}

TEST_CASE("Test environment define cannot redefine built-in function",
          "[environment][define][error]") {
  Environment& env = Environment::getInstance();

  auto fn_name = GENERATE(as<std::string>{}, "print", "len");

  DYNAMIC_SECTION(std::format("Cannot redefine {}", fn_name)) {
    auto captured = helpers::captureStream(
        std::cerr, [&env, &fn_name]() { env.define(fn_name, Object{42.0}); });

    REQUIRE(captured.find(std::format("Cannot assign to {} as it is a function",
                                      fn_name)) != std::string::npos);
  }
}

TEST_CASE("Test environment get returns correct values", "[environment][get]") {
  Environment& env = Environment::getInstance();

  env.define("test_get_var", Object{3.14});
  Object retrieved = env.get(helpers::makeToken("test_get_var"));

  REQUIRE(std::holds_alternative<double>(retrieved));
  REQUIRE(std::get<double>(retrieved) == 3.14);
}

TEST_CASE("Test Environment get errors on undefined variable",
          "[environment][get][error]") {
  Environment& env = Environment::getInstance();

  auto captured = helpers::captureStream(std::cerr, [&env]() {
    env.get(helpers::makeToken("nonexistent_variable_xyz"));
  });

  REQUIRE(captured.find("Undefined variable 'nonexistent_variable_xyz'") !=
          std::string::npos);
}

TEST_CASE("Test environment assign updates existing variable",
          "[environment][assign]") {
  Environment& env = Environment::getInstance();

  env.define("test_assign_var", Object{100.0});

  SECTION("Assign new value of same type") {
    env.assign(helpers::makeToken("test_assign_var"), Object{200.0});
    Object result = env.get(helpers::makeToken("test_assign_var"));
    REQUIRE(std::holds_alternative<double>(result));
    REQUIRE(std::get<double>(result) == 200.0);
  }

  SECTION("Assign value of different type") {
    env.assign(helpers::makeToken("test_assign_var"), Object{"hello"});
    Object result = env.get(helpers::makeToken("test_assign_var"));
    REQUIRE(std::holds_alternative<std::string>(result));
    REQUIRE(std::get<std::string>(result) == "hello");
  }
}

TEST_CASE("Test environment assign errors on undefined variable",
          "[environment][assign][error]") {
  Environment& env = Environment::getInstance();

  auto captured = helpers::captureStream(std::cerr, [&env]() {
    env.assign(helpers::makeToken("undefined_var_abc"), Object{42.0});
  });

  REQUIRE(captured.find("Undefined variable 'undefined_var_abc'") !=
          std::string::npos);
}

TEST_CASE("Test environment assign cannot reassign built-in function",
          "[environment][assign][error]") {
  Environment& env = Environment::getInstance();

  auto fn_name = GENERATE(as<std::string>{}, "print", "len");

  DYNAMIC_SECTION(std::format("Cannot reassign {}", fn_name)) {
    auto captured = helpers::captureStream(std::cerr, [&env, &fn_name]() {
      env.assign(helpers::makeToken(fn_name), Object{42.0});
    });

    REQUIRE(captured.find(std::format("Cannot reassign {} as it is a function",
                                      fn_name)) != std::string::npos);
  }
}

TEST_CASE("Test environment type transitions", "[environment][types]") {
  Environment& env = Environment::getInstance();

  env.define("test_type_var", Object{5.0});

  SECTION("double -> bool") {
    env.assign(helpers::makeToken("test_type_var"), Object{true});
    Object result = env.get(helpers::makeToken("test_type_var"));
    REQUIRE(std::holds_alternative<bool>(result));
  }

  SECTION("double -> string") {
    env.assign(helpers::makeToken("test_type_var"), Object{"test"});
    Object result = env.get(helpers::makeToken("test_type_var"));
    REQUIRE(std::holds_alternative<std::string>(result));
  }

  SECTION("double -> monostate") {
    env.assign(helpers::makeToken("test_type_var"), Object{std::monostate{}});
    Object result = env.get(helpers::makeToken("test_type_var"));
    REQUIRE(std::holds_alternative<std::monostate>(result));
  }
}

TEST_CASE("Test SubcommandHandler exec", "[subcommand][exec]") {
  SubcommandHandler handler({{"test", helpers::testSubcmd}}, "testcmd");

  SECTION("Execute valid subcommand") {
    Object result = handler.exec("test", {"hello"});
    REQUIRE(std::get<std::string>(result) == "hello");
  }

  SECTION("Execute invalid subcommand produces error") {
    auto captured = helpers::captureStream(std::cerr, [&handler]() {
      Object result = handler.exec("invalid", {});
      REQUIRE(std::holds_alternative<std::monostate>(result));
    });
    REQUIRE(captured.find("Subcommand invalid is not valid") !=
            std::string::npos);
  }
}
