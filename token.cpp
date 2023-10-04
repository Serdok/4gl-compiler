//
// Created by lahnin on 04/10/2023.
//

#include "token.hpp"
#include <iomanip>

std::ostream& operator <<(std::ostream& out, const token& token) {
    return out << std::setw(sizeof(token.type)) << static_cast<int>(token.type) << ' ' << std::quoted(token.lexeme, '`');
}
