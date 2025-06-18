#ifndef CAESAR_TOKEN_HPP
#define CAESAR_TOKEN_HPP
#include <iostream>
#include <string>

struct Token {
    int type = 0;
    std::string value;
    int row = 0, col = 0;

    enum TokenType {
        None = 256,
        Number = 257,
        Identifier = 258,
        String = 259,
        Operator = 260,
    };

    Token() : type(None), value("") {}
    Token(const std::string& text, const int row, const int col, const int type) : type(type), value(std::move(text)), row(row), col(col) {}
    ~Token() {
        std::cout << "Deleting Token " << value << std::endl;
    }
};

#endif
