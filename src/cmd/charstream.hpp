#ifndef CAESAR_CHARSTREAM_HPP
#define CAESAR_CHARSTREAM_HPP

#include <string>

class CharStream {
private:
    std::string text;
    int row, col;
public:
    explicit CharStream(const std::string& text);
    int currentRow() const;
    int currentCol() const;
    char currentChar() const;
    char nextChar();
};

#endif