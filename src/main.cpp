#include <fstream>
#include <iostream>

#include "lexer.h"

std::filesystem::path inputPath = "../../asm_examples/hello_world.asm";

int main() {
    inputPath = std::filesystem::absolute(inputPath);
    if (!std::filesystem::exists(inputPath)) {
        std::cout << std::format("File '{}' does not exist!\n", inputPath.string());
        return 1;
    }

    std::ifstream in(inputPath, std::ios::binary);
    if (!in) {
        std::cout << std::format("Failed to open file '{}'\n", inputPath.string());
        return 1;
    }
    std::vector<std::string> inputLines;
    std::string line;
    while (std::getline(in, line)) {
        inputLines.push_back(line);
    }
    std::vector<Token> tokens;
    lex(inputLines, tokens);
    return 0;
}