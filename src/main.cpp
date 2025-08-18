#include <fstream>
#include <iostream>
#include <sstream>

#include "cmd/error.hpp"
#include "cmd/expr.hpp"
#include "cmd/interpreter.hpp"
#include "cmd/parser.hpp"
#include "cmd/scanner.hpp"
#include "cmd/token.hpp"

Err &e = Err::getInstance();

void run(const std::string &src) {
  auto s = Scanner(src);
  const std::vector<Token> tokens = s.scanTokens();
  Parser p = Parser(tokens);
  std::unique_ptr<Expr> expr = p.parse();

  if (e.hadError) return;

  Interpreter interpreter = Interpreter();
  interpreter.interpret(expr);
}

void runFile(const std::string &path) {
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
    if (line.empty()) break;
    run(line);
    e.hadError = false;
  }
}

int main(int argc, char **argv) {
  try {
    if (argc > 2) {
      std::cout << std::format("Usage: {} [script]\n", argv[0]);
      return 64;
    } else if (argc == 2) {
      runFile(argv[1]);
    } else {
      runPrompt();
    }
  } catch (const std::exception &e) {
    std::cerr << std::format("Exception: {}\n", e.what());
    return 1;
  }

  return 0;
}
