//
// Created by lahnin on 02/10/2023.
//

#pragma once

#include <ostream>
#include <string>

enum class token_type {
    end_of_file = std::char_traits<char>::eof(),

    comment,
    string_literal,
    long_number,
    decimal_number,
    identifier,

    // TODO: Reserved keywords as tokens

    // Single tokens
    ampersand = '&',
    backslash = '\\',
    caret = '^',
    close_angle = '>',
    close_bracket = ']',
    close_curly = '}',
    close_paren = ')',
    colon = ':',
    comma = ',',
    dollar = '$',
    dot = '.',
    equal = '=',
    hyphen = '-',
    open_angle = '<',
    open_bracket = '[',
    open_curly = '{',
    open_paren = '(',
    percent = '%',
    pipe = '|',
    plus = '+',
    pound = '#',
    question_mark = '?',
    quotation_mark = '"',
    semicolon = ';',
    slash = '/',
    star = '*',
    underscore = '_',
};

template<typename T, std::same_as<T>... Ts>
constexpr bool is_one_of(T t, T u, Ts... ts) {
    return is_one_of(t, u) or is_one_of(t, ts...);
}

template<typename T>
constexpr bool is_one_of(T t, T u) {
    return t == u;
}

struct token {
    token_type type = token_type::end_of_file;
    std::string lexeme;

    constexpr bool operator ==(const token& other) const { return type == other.type and lexeme == other.lexeme; }

    friend std::ostream& operator <<(std::ostream& out, const token& token);
};

template<typename... Args>
constexpr token make_token(token_type type, Args&&... args) requires std::constructible_from<std::string, Args...> {
    return token{type, std::string(std::forward<Args>(args)...)};
}
