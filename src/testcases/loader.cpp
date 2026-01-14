// SPDX-FileCopyrightText: Copyright 2026 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fstream>
#include <filesystem>

#include "loader.h"
#include "testcase.h"
#include "parser/parser.h"

#define CHECK(is_true, message) if (!is_true ) { LOG_ERROR(message); }

namespace Testcases
{

void loadTest(GlobalState& globalState, const std::filesystem::path& path) {
    u64 fileSize = std::filesystem::file_size(path);
    std::ifstream in(path, std::ios::binary);

    std::vector<char>  fileBuffer(fileSize + 1);
    in.read(fileBuffer.data(), fileSize);
    fileBuffer[fileSize] = '\0';

    ryml::Tree tree = ryml::parse_in_place(fileBuffer.data());
    ryml::ConstNodeRef root = tree.rootref();

    CHECK(root.is_seq(), "Root node must be a sequence of checkpoints");
    for (auto checkpointNode : root.children()) {
        Checkpoint checkpoint{};

        CHECK(checkpointNode.is_map(), "Checkpoint node must be a map");
        checkpointNode["id"] >> checkpoint.id;
        ryml::ConstNodeRef registersNode = checkpointNode["registers"];
        CHECK(registersNode.is_map(), "Registers node must be a map");
        for (const ryml::ConstNodeRef registerNode : registersNode.children()) {
            std::string registerName{ registerNode.key().str, registerNode.key().len };
            if (!globalState.cpu.reg64.contains(registerName) &&
                !globalState.cpu.reg32.contains(registerName) &&
                !globalState.cpu.reg16.contains(registerName) &&
                !globalState.cpu.reg8.contains(registerName)) {
                LOG_ERROR("Unknown register '{}' in testcase!", registerName);
            }
            std::string value{ registerNode.val().str, registerNode.val().len };
            u64 registerValue = textToNumber(value);

            checkpoint.registers[registerName] = registerValue;
        }
        ryml::ConstNodeRef flagsNode = checkpointNode["flags"];
        CHECK(flagsNode.is_map(), "Flags node must be a map");
        for (const ryml::ConstNodeRef flagNode : flagsNode.children()) {
            std::string flagName{ flagNode.key().str, flagNode.key().len };
            std::ranges::transform(flagName, flagName.begin(), ::tolower);
            if (!globalState.cpu.flags.contains(flagName)) {
                LOG_ERROR("Unknown flag '{}' in testcase!", flagName);
            }
            std::string value{ flagNode.val().str, flagNode.val().len };
            if (value == "0") {
                checkpoint.flags[flagName] = false;
            }
            else if (value == "1") {
                checkpoint.flags[flagName] = true;
            }
            else {
                LOG_ERROR("Invalid flag value '{}' for flag '{}' in testcase!", value, flagName);
            }

        }
        if (checkpointNode.has_child("exit")) {
            ryml::ConstNodeRef exitNode = checkpointNode["exit"];
            exitNode >> checkpoint.exit;
        }
        globalState.testcase.checkpoints.push_back(checkpoint);
    }
}

} // namespace Testcases
