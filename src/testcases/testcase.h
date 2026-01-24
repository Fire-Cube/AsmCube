// SPDX-FileCopyrightText: Copyright 2026 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"

namespace Testcases
{

struct Checkpoint {
    u8 id;
    std::unordered_map<std::string, u64> registers;
    std::unordered_map<std::string, bool> flags;
    bool exit = false;
};

struct Test {
    bool testEnabled = false;
    std::vector<Checkpoint> checkpoints;
};

} // namespace Testcases
