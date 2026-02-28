#include <cmd/interpreter.hpp>
#include <cmd/object.hpp>
#include <cmd/parser.hpp>
#include <cmd/scanner.hpp>
#include <cmd/stmnt.hpp>
#include <cmd/token.hpp>
#include <core/context.hpp>
#include <core/target.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <error.hpp>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
// clang-format off
#include <readline/history.h>
#include <readline/readline.h>
// clang-format on
#include <string>
#include <variant>
#include <vector>

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
CmdError& cmdError = CmdError::getInstance();
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
Context& ctx = Context::getInstance();

void run(const std::string& src) {
  auto s = Scanner(src);
  const std::vector<Token> tokens = s.scanTokens();
  if (cmdError.m_had_error) {
    return;  // Stop if scan errors occurred
  }

  Parser p = Parser(tokens);
  std::unique_ptr<Stmnt> const statement = p.parse();
  if (cmdError.m_had_error) {
    return;
  }

  Interpreter interpreter = Interpreter();
  Object const result = interpreter.interpret(statement);
  if (cmdError.m_had_error) {
    return;
  }
  if (!std::holds_alternative<std::monostate>(result)) {
    std::cout << Interpreter::stringify(result) << '\n';
  }
}

bool shouldExit(char** buf) {
  return *buf == nullptr || std::strcmp(*buf, "(null)") == 0;
}

void runPrompt() {
  char* lineBuf = nullptr;
  const char* prompt = "> ";
  std::string line;

  while (true) {
    lineBuf = readline(prompt);
    if (shouldExit(&lineBuf)) {
      clear_history();
      return;
    }
    if (*lineBuf != 0) add_history(lineBuf);
    line = lineBuf;
    free(lineBuf);  // NOLINT(cppcoreguidelines-no-malloc)
    run(line);
    cmdError.m_had_error = false;
  }
}

void runWithFile(const std::string& filePath) {
  if (!std::filesystem::exists(filePath)) {
    std::cout << "Target does not exist!\n";
    runPrompt();
  } else {
    if (Target::isFileValid(filePath)) {
      std::cout << std::format("Target set to {}\n", filePath);
      Context::setTarget(Target::create(filePath));
    } else
      // TODO: Add Mach-O FAT binary magic to Target::isFileValid
      std::cout << "Target is valid but cannot be ran on current platform!\n";
    runPrompt();
  }
}
}  // namespace

int main(int argc, char** argv) {
  if (argc > 2) {
    std::cout << std::format("Usage: {} [file]\n", argv[0]);
    return 64;
  } else if (argc == 2) {
    runWithFile(argv[1]);
  } else {
    runPrompt();
  }
  return 0;
}
