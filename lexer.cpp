//
// Created by lahnin on 02/10/2023.
//

#include "lexer.hpp"
#include <format>

lexical_error::lexical_error(std::string_view message, int line, int column)
: std::logic_error(std::format("lexical error at {}:{} - {}", line, column, message))
{

}

lexer::lexer(std::istream& stream)
: m_stream{stream}, m_line{1}, m_columns{decltype(m_columns)::container_type{1}}
{
    // Irrecoverable stream error
    m_stream.exceptions(std::ios_base::badbit);
}

std::char_traits<char>::char_type lexer::get() {
    auto codepoint = m_stream.get();
    if (std::char_traits<char>::eq(codepoint, '\n')) {
        m_line += 1;
        m_columns.emplace(1);
    } else {
        m_columns.top() += 1;
    }
    return codepoint;
}

void lexer::unget() {
    m_stream.unget();
    auto codepoint = m_stream.peek();
    if (std::char_traits<char>::eq(codepoint, '\n')) {
        m_line -= 1;
        m_columns.pop();
    } else {
        m_columns.top() -= 1;
    }
}

token lexer::next() {
    while (std::isspace(m_stream.peek())) {
        get();
    }

    auto codepoint = m_stream.peek();

    if (m_stream.eof() or std::char_traits<char>::eq(codepoint, std::char_traits<char>::eof())) {
        return make_token(token_type::end_of_file);
    }

    // Look for comments. All characters are consumed until a new line is reached
    if (std::char_traits<char>::eq(codepoint, '|')) {
        return handle_comment();
    }

    // Look for string literals. All characters between quotation marks are consumed
    if (std::char_traits<char>::eq(codepoint, '"')) {
        return handle_string_literal();
    }

    // Look for a number
    if (std::isdigit(codepoint) or std::char_traits<char>::eq(codepoint, '.')) {
        return handle_number();
    }

    // Look for an identifier or reserved keyword
    if (std::isalpha(codepoint)) {
        return handle_identifier();
    }

    // Look for single tokens
    constexpr const std::string_view special_punctuation = "%&()*+,-./:;<=>?[\\]^{}";
    if (special_punctuation.contains(codepoint)) {
        codepoint = get();
        return make_token(token_type{codepoint}, 1, std::char_traits<char>::to_char_type(codepoint));
    }

    mark_error(std::format("unexpected character '{}' (codepoint {})", std::char_traits<char>::to_char_type(codepoint), codepoint));
}

token lexer::handle_comment() {
    auto codepoint = get();
    std::string lexeme;

    // Get everything until a new line
    while (not std::char_traits<char>::eq(codepoint, '\n')) {
        if (std::char_traits<char>::eq(codepoint, std::char_traits<char>::eof())) {
            // No error
            break;
        }
        lexeme.push_back(codepoint);
        codepoint = get();
    }

    return make_token(token_type::comment, lexeme);
}

token lexer::handle_string_literal() {
    auto codepoint = get();
    std::string lexeme;

    if (not std::char_traits<char>::eq(codepoint, '"')) {
        mark_error(std::format("expected '\"' to start a string literal, got '{}' (codepoint {})", std::char_traits<char>::to_char_type(codepoint), codepoint));
    }

    lexeme.push_back(codepoint);
    codepoint = get();
    // Get everything until a matching quotation mark is found
    while (not std::char_traits<char>::eq(codepoint, '"')) {
        if (std::char_traits<char>::eq(codepoint, std::char_traits<char>::eof())) {
            mark_error("unexpected end of file when parsing a string literal");
        }
        lexeme.push_back(codepoint);
        codepoint = get();
    }

    if (not std::char_traits<char>::eq(codepoint, '"')) {
        mark_error(std::format("expected '\"' to end a string literal, got '{}' (codepoint {})", std::char_traits<char>::to_char_type(codepoint), codepoint));
    }
    lexeme.push_back(codepoint);

    return make_token(token_type::string_literal, lexeme);
}

token lexer::handle_number() {
    auto codepoint = get();
    std::string lexeme;

    if (not (std::isdigit(codepoint) or std::char_traits<char>::eq(codepoint, '.'))) {
        mark_error(std::format("expected a digit or '.', got {}", codepoint));
    }

    // Handle hexadecimal notation
    if (std::char_traits<char>::eq(codepoint, '0')
    and std::char_traits<char>::eq(m_stream.peek(), 'x')) {
        lexeme.push_back(codepoint);
        lexeme.push_back(get());
        return handle_hexadecimal_number(std::move(lexeme));
    }

    while (std::isdigit(codepoint)) {
        lexeme.push_back(codepoint);
        codepoint = get();
    }

    // Allow one decimal point (it may be the starting point if there were no digits before)
    if (std::char_traits<char>::eq(codepoint, '.')) {
        lexeme.push_back(codepoint);
        return handle_decimal(lexeme);
    }

    // Allow exponents
    // TODO: should it still be considered as a long?
    if (std::char_traits<char>::eq(codepoint, 'e')) {
        lexeme.push_back(codepoint);
        return handle_exponent(lexeme);
    }

    if (std::isalpha(codepoint)) {
        lexeme.push_back(codepoint);
        mark_error(std::format("malformed number {}", lexeme));
    }

    unget();
    return make_token(token_type::long_number, lexeme);
}

token lexer::handle_hexadecimal_number(std::string lexeme) {
    auto codepoint = get();
    if (not std::isxdigit(codepoint)) {
        mark_error(std::format("expected a hexadecimal digit, got {}", codepoint));
    }

    // Every character up to this point must be a hexadecimal representation
    while (std::isxdigit(codepoint)) {
        lexeme.push_back(codepoint);
        codepoint = get();
    }

    unget();
    return make_token(token_type::long_number, std::move(lexeme));
}

token lexer::handle_decimal(std::string lexeme) {
    auto codepoint = get();

    // Get all digits after the decimal point
    while (std::isdigit(codepoint)) {
        lexeme.push_back(codepoint);
        codepoint = get();
    }
    if (lexeme.ends_with('.')) {
        mark_error("no digits found after decimal point");
    }
    if (std::char_traits<char>::eq(codepoint, '.')) {
        mark_error("only one decimal point is allowed");
    }

    // Handle exponents
    if (std::char_traits<char>::eq(codepoint, 'e')) {
        lexeme.push_back(codepoint);
        return handle_exponent(std::move(lexeme));
    }

    unget();
    return make_token(token_type::decimal_number, std::move(lexeme));
}

token lexer::handle_exponent(std::string lexeme) {
    auto codepoint = get();
    if (not std::isdigit(codepoint) or std::char_traits<char>::eq(codepoint, '-')) {
        mark_error(std::format("expected exponent digit, got {}", codepoint));
    }

    // Get all exponent digits
    if (std::char_traits<char>::eq(codepoint, '-') or std::char_traits<char>::eq(codepoint, '+')) {
        lexeme.push_back(codepoint);
        codepoint = get();
    }
    while (std::isdigit(codepoint)) {
        lexeme.push_back(codepoint);
        codepoint = get();
    }

    if (lexeme.ends_with('e') or lexeme.ends_with('-') or lexeme.ends_with('+')) {
        mark_error("no digits found after exponent");
    }

    unget();
    return make_token(token_type::decimal_number, std::move(lexeme));
}

token lexer::handle_identifier() {
    auto codepoint = get();
    std::string lexeme;

    if (not std::isalpha(codepoint)) {
        mark_error(std::format("expected an alphabetical character, got {}", codepoint));
    }

    // Look for zero or more letters or digits (allow dots)
    while (std::isalnum(codepoint) or std::char_traits<char>::eq(codepoint, '.')) {
        lexeme.push_back(codepoint);
        codepoint = get();
    }

    // TODO: Check for reserved keywords?


    // If we get here it means what we got is truly an identifier
    unget();
    return make_token(token_type::identifier, std::move(lexeme));
}

void lexer::mark_error(std::string_view message) {
    throw lexical_error(message, m_line, m_columns.top());
}
