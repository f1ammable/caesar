#include <cmd/interpreter.hpp>
#include <cmd/object.hpp>
#include <cmd/parser.hpp>
#include <cmd/scanner.hpp>
#include <cmd/stmnt.hpp>
#include <cmd/token.hpp>
#include <core/context.hpp>
#include <core/target.hpp>
#include <cstdlib>
#include <error.hpp>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
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

void runPrompt() {
  std::string line;

  while (true) {
    line.clear();
    std::cout << "> ";
    std::cout.flush();
    if (!std::getline(std::cin, line)) {
      break;
    }
    if (line.empty()) {
      continue;
    }
    run(line);
    cmdError.m_had_error = false;
  }
  std::cout << '\n';
}

void runFile(const std::string& filePath) {
  if (!std::filesystem::exists(filePath)) {
    std::cout << "Target does not exist!\n";
    runPrompt();
  } else {
    if (Target::isFileValid(filePath)) {
      std::cout << std::format("Target set to {}\n", filePath);
      Target::create(filePath);
    } else
      // TODO: Add Mach-O FAT binary magic to Target::isFileValid
      std::cout << "Target is valid but cannot be ran on current platform!\n";
    runPrompt();
  }
}
}  // namespace

int main(int argc, char** argv) {
  if (argc > 2) {
    std::cout << std::format("Usage: {} [script]\n", argv[0]);
    return 64;
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    runPrompt();
  }
  return 0;
}

// int main(int argc, char** argv) {
//   if (argc > 2) {
//     std::cout << "Usage: caesar [file]" << std::endl;
//     return 1;
//   }
//
//   std::ifstream f(argv[1], std::ios::in | std::ios::binary);
//
//   if (f.fail()) {
//     std::cout << "File does not exist" << std::endl;
//     f.close();
//     return 1;
//   }
//
//   Macho m = Macho(f);
//   m.dump();
//   f.close();
// }
