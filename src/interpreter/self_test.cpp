// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_state.h"
#include "interpreter/interpreter.h"

void selfTestCPU() {
    GlobalState globalState{};
    globalState.cpu.rax = 0x1234567890ABCDEF;
    if (globalState.cpu.eax != 0x90ABCDEF) {
        LOG_ERROR("Self-test failed: EAX register value mismatch!");
    }
    if (globalState.cpu.ax != 0xCDEF) {
        LOG_ERROR("Self-test failed: AX register value mismatch!");
    }
    if (globalState.cpu.ah != 0xCD) {
        LOG_ERROR("Self-test failed: AH register value mismatch!");
    }
    if (globalState.cpu.al != 0xEF) {
        LOG_ERROR("Self-test failed: AL register value mismatch!");
    }
    Operand operand1{ Register{.name="%eax"} };
    writeOperand(operand1, 0xDEADBEEF, globalState);
    std::string targetSize{"q"};
    Operand operand2{ Register{.name="%rax"} };
    u64 value = readOperand(operand2, targetSize, globalState);
    if (value != 0x00000000DEADBEEF) {
        LOG_ERROR("Self-test failed: Result of write to eax is not correct!");
    }
    LOG_DEBUG("Self-test success!");
}