#ifndef CAESAR_SCANNER_HPP
#define CAESAR_SCANNER_HPP

#include <memory>

#include "token.hpp"
#include "charstream.hpp"

class Scanner {
private:
    CharStream& m_cs;
    std::shared_ptr<Token> m_token;

    void initToken();
    void setText(const std::string &text);
    void setType(int type);
    void enterChar();
    bool intLiteral();
    bool numLiteral();

public:
    explicit Scanner(CharStream &cs);
    std::shared_ptr<Token> currentToken() const;
    std::shared_ptr<Token> nextToken();
    char currentChar() const;
    char nextChar();
    bool isWhitespace(char c) const;
    void skipWhitespace();
};


#endif