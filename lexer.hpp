//
// Created by lahnin on 02/10/2023.
//

#pragma once

#include "token.hpp"
#include <istream>
#include <stack>
#include <stdexcept>

class lexical_error : public std::logic_error {
public:
    lexical_error(std::string_view message, int line, int column);
};

class lexer {
public:
    explicit lexer(std::istream& stream);

    token next();

private:
    std::char_traits<char>::char_type get();
    void unget();

    [[noreturn]] void mark_error(std::string_view message);

    token handle_comment();
    token handle_string_literal();
    token handle_number();
    token handle_hexadecimal_number(std::string lexeme);
    token handle_decimal(std::string lexeme);
    token handle_exponent(std::string lexeme);
    token handle_identifier();

private:
    std::istream& m_stream;
    int m_line;
    std::stack<int> m_columns;
};
