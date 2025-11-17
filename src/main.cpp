#include <chrono>
#include <fstream>
#include <iostream>

#include <argparse/argparse.hpp>

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include "logging.h"

#include "lexer/lexer.h"
#include "parser/parser.h"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser argumentParser("AsmCube");

    argumentParser.add_argument("inputFile")
        .help("path to input file")
        .required();

    argumentParser.add_argument("--dump")
        .help("dumps the output by lexer and parser as json to separate files")
        .default_value(false)
        .implicit_value(true);

    argumentParser.add_argument("--logLevel")
        .help("log level (error, warning, info, debug)")
        .default_value(std::string("info"))
        .action([](const std::string& value)
        {
            if (value == "error") {
                logLevel = LogLevel::Error;
            }
            else if (value == "warning") {
                logLevel = LogLevel::Warning;
            }
            else if (value == "info") {
                logLevel = LogLevel::Info;
            }
            else if (value == "debug") {
                logLevel = LogLevel::Debug;
            }
            else {
                throw std::runtime_error("Invalid log level: " + value);
            }
        }
        );

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
        LOG_ERROR("File '{}' does not exist!", inputPath.string());
        return 1;
    }

    std::ifstream in(inputPath, std::ios::binary);
    if (!in) {
        LOG_ERROR("Failed to open file '{}'", inputPath.string());
        return 1;
    }

    std::vector<std::string> inputLines;
    std::string line;
    while (std::getline(in, line)) {
        inputLines.push_back(line);
    }

    std::vector<Token> tokens;
    auto startTime = std::chrono::high_resolution_clock::now();
    lex(inputLines, tokens);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto lexDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() / 1'000'000.;
    LOG_DEBUG("Lexing completed in {} ms, {} tokens generated.", lexDuration, tokens.size());

    if (argumentParser["--dump"] == true) {
        const auto outputPath = std::filesystem::absolute("lex.json");
        std::ofstream out(outputPath, std::ios::binary);
        cereal::JSONOutputArchive archive(out);
        archive(cereal::make_nvp("tokens", tokens));
        LOG_INFO("Lexed tokens dumped to '{}'.", outputPath.string());
    }

    std::vector<Section> ast;
    startTime = std::chrono::high_resolution_clock::now();
    parse(tokens, ast);
    endTime = std::chrono::high_resolution_clock::now();
    lexDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() / 1'000'000.;
    LOG_DEBUG("Parsing completed in {} ms.", lexDuration);

    if (argumentParser["--dump"] == true) {
        const auto outputPath = std::filesystem::absolute("ast.json");
        std::ofstream out(outputPath, std::ios::binary);
        cereal::JSONOutputArchive archive(out);
        archive(cereal::make_nvp("ast", ast));
        LOG_INFO("AST dumped to '{}'.", outputPath.string());
    }

    return 0;
}