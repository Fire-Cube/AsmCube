// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global_state.h"
#include "interpreter/interpreter.h"


void selfTestCPU() {
    GlobalState globalState{};
    Ast::Operand operandRAX{ Parser::registerTable.at("rax") };
    Ast::Operand operandEAX{ Parser::registerTable.at("eax") };
    Ast::Operand operandAX{ Parser::registerTable.at("ax") };
    Ast::Operand operandAH{ Parser::registerTable.at("ah") };
    Ast::Operand operandAL{ Parser::registerTable.at("al") };

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

    if (Interpreter::readOperand(operandRAX, Ast::Width::Quad, globalState) != 0x1234567890ABCDEF) {
        LOG_ERROR("Self-test failed: RAX readOperand value mismatch!");
    }
    if (Interpreter::readOperand(operandEAX, Ast::Width::Long, globalState) != 0x90ABCDEF) {
        LOG_ERROR("Self-test failed: EAX readOperand value mismatch!");
    }
    if (Interpreter::readOperand(operandAX, Ast::Width::Word, globalState) != 0xCDEF) {
        LOG_ERROR("Self-test failed: AX readOperand value mismatch!");
    }
    if (Interpreter::readOperand(operandAH, Ast::Width::Byte, globalState) != 0xCD) {
        LOG_ERROR("Self-test failed: AH readOperand value mismatch!");
    }
    if (Interpreter::readOperand(operandAL, Ast::Width::Byte, globalState) != 0xEF) {
        LOG_ERROR("Self-test failed: AL readOperand value mismatch!");
    }

    auto resetRAX = [&](u64 value=0) {
        globalState.cpu.rax = value;
        if (globalState.cpu.rax != value) {
            LOG_ERROR("Self-test failed: RAX direct register access write value mismatch!");
        }
    };

    resetRAX();
    Interpreter::writeOperand(operandRAX, 0xDEADDEADDEAD, Ast::Width::Quad, globalState);
    if (globalState.cpu.rax != 0xDEADDEADDEAD) {
        LOG_ERROR("Self-test failed: RAX writeOperand did not update RAX correctly!");
    }

    globalState.cpu.rax = 0x1234567890ABCDEF;
    Interpreter::writeOperand(operandEAX, 0xDEADBEEF, Ast::Width::Long, globalState);
    if (globalState.cpu.rax != 0x00000000DEADBEEF) {
        LOG_ERROR("Self-test failed: EAX writeOperand did not update RAX correctly!");
    }

    resetRAX(0x1234567890ABCDEF);
    Interpreter::writeOperand(operandAX, 0xBEEF, Ast::Width::Word, globalState);
    if (globalState.cpu.rax != 0x1234567890ABBEEF) {
        LOG_ERROR("Self-test failed: AX writeOperand did not update RAX correctly!");
    }

    resetRAX(0x1234567890ABCDEF);
    Interpreter::writeOperand(operandAH, 0xAD, Ast::Width::Byte, globalState);
    if (globalState.cpu.rax != 0x1234567890ABADEF) {
        LOG_ERROR("Self-test failed: AH writeOperand did not update RAX correctly!");
    }

    resetRAX(0x1234567890ABCDEF);
    Interpreter::writeOperand(operandAL, 0xA0, Ast::Width::Byte, globalState);
    if (globalState.cpu.rax != 0x1234567890ABCDA0) {
        LOG_ERROR("Self-test failed: AL writeOperand did not update RAX correctly!");
    }

    LOG_DEBUG("Self-test success!");
}
