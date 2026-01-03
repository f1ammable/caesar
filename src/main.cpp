#include <cstdlib>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "cmd/error.hpp"
#include "cmd/interpreter.hpp"
#include "cmd/parser.hpp"
#include "cmd/scanner.hpp"
#include "cmd/token.hpp"
#include "object.hpp"
#include "stmnt.hpp"

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
Error& e = Error::getInstance();

void run(const std::string& src) {
  auto s = Scanner(src);
  const std::vector<Token> tokens = s.scanTokens();
  if (e.had_error) {
    return;  // Stop if scan errors occurred
  }

  Parser p = Parser(tokens);
  std::unique_ptr<Stmnt> const statement = p.parse();
  if (e.had_error) {
    return;
  }

  Interpreter interpreter = Interpreter();
  Object const result = interpreter.interpret(statement);
  if (e.had_error) {
    return;
  }
  if (!std::holds_alternative<std::monostate>(result)) {
    std::cout << Interpreter::stringify(result) << '\n';
  }
}

void runFile(const std::string& path) {
  std::ifstream const file(path);
  std::stringstream buffer;
  buffer << file.rdbuf();
  run(buffer.str());
  if (e.had_error) {
    exit(65);  // NOLINT(concurrency-mt-unsafe)
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
    e.had_error = false;
  }
  std::cout << '\n';
}
}  // namespace

int main(int argc, char** argv) {
  try {
    if (argc > 2) {
      std::cout << std::format("Usage: {} [script]\n", argv[0]);
      return 64;
    } else if (argc == 2) {
      runFile(argv[1]);
    } else {
      runPrompt();
    }
  } catch (const std::exception& e) {
    std::cerr << std::format("Exception: {}\n", e.what());
    return 1;
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
