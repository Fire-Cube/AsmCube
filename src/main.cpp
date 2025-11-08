#include <fstream>
#include <iostream>

#include <argparse/argparse.hpp>

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include "lexer.h"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser argumentParser("AsmCube");

    argumentParser.add_argument("inputFile")
        .help("path to input file")
        .required();

    argumentParser.add_argument("--dump")
        .help("dumps the output by lexer and parser as json to separate files")
        .default_value(false)
        .implicit_value(true);

    try {
        argumentParser.parse_args(argc, argv);
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        std::cerr << argumentParser;
        std::exit(1);
    }

    std::filesystem::path inputPath = std::filesystem::absolute(argumentParser.get<std::string>("inputFile"));
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

    if (argumentParser["--dump"] == true) {
        std::ofstream out("lex.json", std::ios::binary);
        cereal::JSONOutputArchive archive(out);
        archive(cereal::make_nvp("tokens", tokens));
    }

    return 0;
}