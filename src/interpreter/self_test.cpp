// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_state.h"
#include "interpreter/interpreter.h"

void selfTestCPU() {
    GlobalState globalState{};
    Operand operandRAX{ Register{.name="%rax"} };
    Operand operandEAX{ Register{.name="%eax"} };
    Operand operandAX{ Register{.name="%ax"} };
    Operand operandAH{ Register{.name="%ah"} };
    Operand operandAL{ Register{.name="%al"} };
    
    std::string targetSizeQuad = "q";
    std::string targetSizeLong = "l";
    std::string targetSizeWord = "w";
    std::string targetSizeByte = "b";

    globalState.cpu.rax = 0x1234567890ABCDEF;
    if (globalState.cpu.eax != 0x90ABCDEF) {
        LOG_ERROR("Self-test failed: EAX direct register access read value mismatch!");
    }
    if (globalState.cpu.ax != 0xCDEF) {
        LOG_ERROR("Self-test failed: AX direct register access read value mismatch!");
    }
    if (globalState.cpu.ah != 0xCD) {
        LOG_ERROR("Self-test failed: AH direct register access read value mismatch!");
    }
    if (globalState.cpu.al != 0xEF) {
        LOG_ERROR("Self-test failed: AL direct register access read value mismatch!");
    }

    if (readOperand(operandRAX, targetSizeQuad, globalState) != 0x1234567890ABCDEF) {
        LOG_ERROR("Self-test failed: RAX readOperand value mismatch!");
    }
    if (readOperand(operandEAX, targetSizeLong, globalState) != 0x90ABCDEF) {
        LOG_ERROR("Self-test failed: EAX readOperand value mismatch!");
    }
    if (readOperand(operandAX, targetSizeWord, globalState) != 0xCDEF) {
        LOG_ERROR("Self-test failed: AX readOperand value mismatch!");
    }
    if (readOperand(operandAH, targetSizeByte, globalState) != 0xCD) {
        LOG_ERROR("Self-test failed: AH readOperand value mismatch!");
    }
    if (readOperand(operandAL, targetSizeByte, globalState) != 0xEF) {
        LOG_ERROR("Self-test failed: AL readOperand value mismatch!");
    }

    auto resetRAX = [&](u64 value=0) {
        globalState.cpu.rax = value;
        if (globalState.cpu.rax != value) {
            LOG_ERROR("Self-test failed: RAX direct register access write value mismatch!");
        }
    };

    resetRAX();
    writeOperand(operandRAX, 0xDEADDEADDEAD, globalState);
    if (globalState.cpu.rax != 0xDEADDEADDEAD) {
        LOG_ERROR("Self-test failed: RAX writeOperand did not update RAX correctly!");
    }

    globalState.cpu.rax = 0x1234567890ABCDEF;
    writeOperand(operandEAX, 0xDEADBEEF, globalState);
    if (globalState.cpu.rax != 0x00000000DEADBEEF) {
        LOG_ERROR("Self-test failed: EAX writeOperand did not update RAX correctly!");
    }

    resetRAX(0x1234567890ABCDEF);
    writeOperand(operandAX, 0xBEEF, globalState);
    if (globalState.cpu.rax != 0x1234567890ABBEEF) {
        LOG_ERROR("Self-test failed: AX writeOperand did not update RAX correctly!");
    }

    resetRAX(0x1234567890ABCDEF);
    writeOperand(operandAH, 0xAD, globalState);
    if (globalState.cpu.rax != 0x1234567890ABADEF) {
        LOG_ERROR("Self-test failed: AH writeOperand did not update RAX correctly!");
    }

    resetRAX(0x1234567890ABCDEF);
    writeOperand(operandAL, 0xA0, globalState);
    if (globalState.cpu.rax != 0x1234567890ABCDA0) {
        LOG_ERROR("Self-test failed: AL writeOperand did not update RAX correctly!");
    }

    LOG_DEBUG("Self-test success!");
}