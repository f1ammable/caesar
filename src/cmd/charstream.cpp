#include "charstream.hpp"

CharStream::CharStream(const std::string &text) : text(text), row(1), col(1) {}

int CharStream::currentRow() const {
    return row;
}

int CharStream::currentCol() const {
    return col;
}

char CharStream::currentChar() const {
   if (text.empty())
       return EOF;

    if (col > text.length())
        return EOF;

    return text[col-1];
}

char CharStream::nextChar() {
    ++col;
    return currentChar();
}



