#include <fstream>
#include <iostream>
#include <sstream>

#include "cmd/error.hpp"
#include "cmd/interpreter.hpp"
#include "cmd/parser.hpp"
#include "cmd/scanner.hpp"
#include "cmd/token.hpp"

Error& e = Error::getInstance();

void run(const std::string& src) {
  auto s = Scanner(src);
  const std::vector<Token> tokens = s.scanTokens();
  Parser p = Parser(tokens);
  std::unique_ptr<Stmnt> statement = p.parse();
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
}

void runPrompt() {
  std::string line;

  while (true) {
    line.clear();
    std::cout << "> ";
    std::cout.flush();
    if (!std::getline(std::cin, line)) break;  // Exit on EOF or read error
    if (line.empty()) continue;
    run(line);
    e.hadError = false;
  }
  std::cout << std::endl;  // Print newline after EOF
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
