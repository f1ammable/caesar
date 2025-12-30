#include <fstream>
#include <iostream>
#include <sstream>

#include "cmd/error.hpp"
#include "cmd/interpreter.hpp"
#include "cmd/parser.hpp"
#include "cmd/scanner.hpp"
#include "cmd/token.hpp"

Err& e = Err::getInstance();

void run(const std::string& src) {
  auto s = Scanner(src);
  const std::vector<Token> tokens = s.scanTokens();
  std::unique_ptr<Stmnt> statement{};
  // TODO: Remove exceptions entirely
  try {
    Parser p = Parser(tokens);
    statement = p.parse();
  } catch (ParseError& err) {
  }
  if (e.hadError) return;

  Interpreter interpreter = Interpreter();
  interpreter.interpret(statement);
}

void runFile(const std::string& path) {
  std::ifstream file(path);
  std::stringstream buffer;
  buffer << file.rdbuf();
  run(buffer.str());
  if (e.hadError) exit(65);
  if (e.hadRuntimeError) exit(70);
}

void runPrompt() {
  std::string line;

  for (;;) {
    line.clear();
    std::cout << "> ";
    std::getline(std::cin, line);
    if (line.empty()) continue;
    run(line);
    e.hadError = false;
  }
}

// lol
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
