#include "lexer.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    return {std::istreambuf_iterator<char>(file), {}};
}

int main(int argc, char** argv) {
    try {
        [[maybe_unused]] std::vector<std::string> arguments{argv, std::next(argv, argc)};
        const auto source = read_file("main.bc");

        // Lex
        std::ifstream file("main.bc");
        lexer lexer(file);

        for (auto tok = lexer.next(); tok != token{}; tok = lexer.next()) {
            std::cout << tok << std::endl;
        }

        // Parse


        return 0;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
