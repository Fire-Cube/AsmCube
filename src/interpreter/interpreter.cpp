// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>

#include "parser/parser.h"
#include "registers.h"
#include "memory.h"
#include "interpreter.h"
#include "syscalls.h"
#include "mnemonics.h"

namespace Interpreter
{

u64 resolveMemory(const Ast::Memory& memory, GlobalState& globalState) {
    u64 displacement = 0;
    u64 base = 0;
    u64 index = 0;
    u64 scale = 1;

    if (memory.disp.has_value()) {
        if (std::holds_alternative<s64>(*memory.disp)) {
            displacement = std::get<s64>(*memory.disp);
        }
        else if (std::holds_alternative<Ast::Label>(*memory.disp)) {
            displacement = globalState.symbolTable.findSymbol(std::get<Ast::Label>(*memory.disp).name).address;
            if (memory.base.has_value() && memory.base.value().name == "rip") {
                displacement -= globalState.cpu.rip + 8;
            }
        }
    }

    if (memory.base.has_value()) {
        if (memory.base.value().name == "rip") {
            base = globalState.cpu.rip + 8;
        }
        else {
            base = *globalState.cpu.reg64.at(memory.base.value().name);
        }
    }

    if (memory.index.has_value()) {
        index = *globalState.cpu.reg64.at(memory.index.value().name);
    }

    if (memory.scale.has_value()) {
        switch (memory.scale.value()) {
            case Ast::Scale::One:
                scale = 1;
                break;

            case Ast::Scale::Two:
                scale = 2;
                break;

            case Ast::Scale::Four:
                scale = 4;
                break;

            case Ast::Scale::Eight:
                scale = 8;
                break;
        }
    }

    const u64 address = displacement + base + (index * scale);
    return address;
}

std::string getRegisterSize(const CPU& cpu, const std::string& registerName) {
    if (cpu.reg64.find(registerName) != cpu.reg64.end()) {;
        return "q";
    }
    if (cpu.reg32.find(registerName) != cpu.reg32.end()) {
        return "l";
    }
    if (cpu.reg16.find(registerName) != cpu.reg16.end()) {
        return "w";
    }
    if (cpu.reg8.find(registerName) != cpu.reg8.end()) {
        return "b";
    }
    LOG_ERROR("Unknown register '{}'", registerName);
}

std::string getOperandSize(const Ast::Operand& left, const CPU& cpu, const std::string& sizeSuffix) {
    if (std::holds_alternative<Ast::Register>(left)) {
        auto& reg = std::get<Ast::Register>(left);
        auto regSize = getRegisterSize(cpu, reg.name);
        if (!sizeSuffix.empty() && sizeSuffix != regSize) {
            LOG_ERROR("Size suffix does not match register size");
        }
        return regSize;
    }
    if (std::holds_alternative<Ast::Memory>(left)) {
        if (!sizeSuffix.empty()) {
            return sizeSuffix;
        }
        LOG_ERROR("Size suffix is required when operand is memory");
    }
    LOG_ERROR("Cannot determine operand size");
}

std::string getOperandSize(const Ast::Operand& left, const Ast::Operand& right, const CPU& cpu, const std::string& sizeSuffix) {
    if (std::holds_alternative<Ast::Immediate>(right)) {
        LOG_ERROR("Destination cannot be an immediate");
    }
    if (std::holds_alternative<Ast::Register>(right)) {
        auto& dstReg = std::get<Ast::Register>(right);
        auto dstSize = getRegisterSize(cpu, dstReg.name);
        if (std::holds_alternative<Ast::Register>(left)) {
            auto& srcReg = std::get<Ast::Register>(left);
            auto srcSize = getRegisterSize(cpu, srcReg.name);
            if (dstSize != srcSize) {
                LOG_ERROR("Destination register size is different than source register size");
            }
        }
        if (!sizeSuffix.empty() && sizeSuffix != dstSize) {
            LOG_ERROR("Size suffix does not match destination register size");
        }
        return dstSize;
    }
    if (std::holds_alternative<Ast::Memory>(right)) {
        if (std::holds_alternative<Ast::Register>(left)) {
            auto& reg = std::get<Ast::Register>(left);
            return getRegisterSize(cpu, reg.name);
        }
        if (std::holds_alternative<Ast::Memory>(left)) {
            LOG_ERROR("Memory can not be in both src and dst operands");
        }
        if (std::holds_alternative<Ast::Immediate>(left)) {
            if (!sizeSuffix.empty()) {
                return sizeSuffix;
            }
            LOG_ERROR("Size suffix is required when moving immediate to memory");
        }

    }
    LOG_ERROR("Unable to determine operand size");

}

u64 readOperand(const Ast::Operand& operand, std::string& targetSize, GlobalState& globalState) {
    switch (static_cast<Ast::OperandType>(operand.index())) {
        case Ast::OperandType::Register:
            {
                auto& reg = std::get<Ast::Register>(operand);
                std::string regName = reg.name;
                if (globalState.cpu.reg64.contains(regName)) {
                    return *globalState.cpu.reg64[regName];
                }
                if (globalState.cpu.reg32.contains(regName)) {
                    return *globalState.cpu.reg32[regName];
                }
                if (globalState.cpu.reg16.contains(regName)) {
                    return *globalState.cpu.reg16[regName];
                }
                if (globalState.cpu.reg8.contains(regName)) {
                    return *globalState.cpu.reg8[regName];
                }
                LOG_ERROR("Unknown register '{}'", regName);
            }

        case Ast::OperandType::Immediate:
            return std::get<Ast::Immediate>(operand).value;

        case Ast::OperandType::RelativeImmediate:
            {
                const auto& relative = std::get<Ast::RelativeImmediate>(operand);
                if (std::holds_alternative<Ast::Label>(relative.target)) {
                    return globalState.symbolTable.findSymbol(std::get<Ast::Label>(relative.target).name).address;
                }

                return globalState.cpu.rip + 8 + std::get<s64>(relative.target);
            }

        case Ast::OperandType::Symbol:
            {
                const std::string& name = std::get<Ast::Symbol>(operand).name;
                    for (const auto& symbolImmediate : globalState.symbolImmediates) {
                        if (symbolImmediate.name == name) {
                            return symbolImmediate.value;
                        }
                    }
                    for (const auto& [symbolName, symbol] : globalState.symbolTable.symbols) {
                        if (symbolName == name) {
                            return symbol.address;
                        }
                        if (auto pos = name.find('@'); pos != std::string::npos) {
                            std::string baseSymbolName = name.substr(0, pos);
                            if (symbolName == baseSymbolName) {
                                return symbol.address;
                            }
                        }
                    }
                    LOG_ERROR("Unknown symbol {}", name);
                }
                break;

        case Ast::OperandType::Memory:
            {
                auto& memoryOperand = std::get<Ast::Memory>(operand);
                u64 address = resolveMemory(memoryOperand, globalState);
                u64 value = 0;
                switch (targetSize.c_str()[0]) {
                    case 'b':
                        globalState.memory.readMemory<u8>(address, reinterpret_cast<u8&>(value));
                        break;

                    case 'w':
                        globalState.memory.readMemory<u16>(address, reinterpret_cast<u16&>(value));
                        break;

                    case 'l':
                        globalState.memory.readMemory<u32>(address, reinterpret_cast<u32&>(value));
                        break;

                    case 'q':
                        globalState.memory.readMemory<u64>(address, value);
                        break;
                }
                return value;
            }
    }
    LOG_ERROR("Unhandled operand type in readOperand");
}

void writeOperand(const Ast::Operand& operand, const u64 value, std::string& targetSize, GlobalState& globalState) {
    switch (static_cast<Ast::OperandType>(operand.index())) {
        case Ast::OperandType::Register:
            {
                // Register
                auto& reg = std::get<Ast::Register>(operand);
                std::string regName = reg.name;
                if (globalState.cpu.reg64.contains(regName)) {
                    *globalState.cpu.reg64[regName] = value;
                    break;
                }
                if (globalState.cpu.reg32.contains(regName)) {
                    *reinterpret_cast<u64*>(globalState.cpu.reg32[regName]) = value;
                    break;
                }
                if (globalState.cpu.reg16.contains(regName)) {
                    *globalState.cpu.reg16[regName] = static_cast<u16>(value);
                    break;
                }
                if (globalState.cpu.reg8.contains(regName)) {
                    *globalState.cpu.reg8[regName] = static_cast<u8>(value);
                    break;
                }
                LOG_ERROR("Unknown register '{}'", regName);
            }

        case Ast::OperandType::Memory:
            {
                // Memory
                auto& memoryOperand = std::get<Ast::Memory>(operand);
                u64 address = resolveMemory(memoryOperand, globalState);
                switch (targetSize.c_str()[0]) {
                    case 'b':
                        globalState.memory.writeMemory<u8>(address, static_cast<u8>(value));
                        break;

                    case 'w':
                        globalState.memory.writeMemory<u16>(address, static_cast<u16>(value));
                        break;

                    case 'l':
                        globalState.memory.writeMemory<u32>(address, static_cast<u32>(value));
                        break;

                    case 'q':
                        globalState.memory.writeMemory<u64>(address, value);
                        break;
                }
                break;
            }
        default:
            LOG_ERROR("Cannot write to this operand type");
    }
}

std::vector<u8> decodeAscii(const std::string& text) {
    std::vector<u8> result;
    for (u32 i = 0; i < text.size(); ++i) {
        if (text[i] == '\\') {
            switch (text[i + 1]) {
                case 'n':
                    result.push_back('\n');
                    ++i;
                    break;

                case '0':
                    result.push_back('\0');
                    ++i;
                    break;

                default:
                    LOG_ERROR("Unknown escape sequence in string '{}'", text);
            }
        }
        else {
            result.push_back(static_cast<u8>(text[i]));
        }
    }
    return result;
}

int run(Ast::Ast& ast, GlobalState& globalState) {
    std::unordered_map<u64, Ast::Instruction> instructionMap{};
    u64 instructionID = 0;

    // Linking
    LOG_DEBUG("Start linking...");
    auto startTime = std::chrono::high_resolution_clock::now();

    Permission permission{};
    for (Ast::Section& section : ast) {
        if (section.name[0] == '.') {
            section.name = section.name.substr(1);
        }
        if (section.name == "rodata" || section.name.rfind("rodata.") == 0) {
            permission = Permission{ true, false, false };
        }
        else if (section.name == "data" || section.name.rfind("data.") == 0) {
            permission = Permission{ true, true, false };
        }
        else if (section.name == "bss" || section.name.rfind("bss.") == 0) {
            permission = Permission{ true, true, false };
        }
        else if (section.name == "text" || section.name.rfind("text.") == 0) {
            permission = Permission{ true, false, true };
        }
        else {
            LOG_INFO("Unknown section name '{}'", section.name);
        }
        std::string actualSymbolName;
        for (const auto& item : section.items) {
            switch (item.index()) {
                case 0:
                    {
                        // Label
                        actualSymbolName = std::get<Ast::Label>(item).name;
                        break;
                    }

                case 1:
                    {
                        // Directive
                        const Ast::Directive& directive = std::get<Ast::Directive>(item);
                        switch (directive.name) {
                            case Ast::Directive::Name::ascii:
                                {
                                    auto buffer = decodeAscii(directive.arguments[0]);
                                    Symbol& symbol = globalState.symbolTable.addSymbol(actualSymbolName,buffer.size());
                                    for (u64 i = 0; i < buffer.size(); ++i) {
                                        globalState.memory.writeMemoryNoExcept(symbol.address + i, buffer[i]);
                                    }
                                    globalState.memory.setPermission(symbol.address, buffer.size(), permission);
                                }
                                break;

                            case Ast::Directive::Name::asciz:
                                {
                                    auto buffer = decodeAscii(directive.arguments[0]);
                                    buffer.push_back('\0');
                                    Symbol& symbol = globalState.symbolTable.addSymbol(actualSymbolName,buffer.size());
                                    for (u64 i = 0; i < buffer.size(); ++i) {
                                        globalState.memory.writeMemoryNoExcept(symbol.address + i, buffer[i]);
                                    }
                                    globalState.memory.setPermission(symbol.address, buffer.size(), permission);
                                }
                                break;

                            case Ast::Directive::Name::skip:
                            case Ast::Directive::Name::space:
                                {
                                    u32 size = std::stoull(directive.arguments[0]);
                                    u64 data = 0u;
                                    if (directive.arguments.size() > 1) {
                                        data = Parser::textToNumber(directive.arguments[1]);
                                    }
                                    Symbol& symbol = globalState.symbolTable.addSymbol(actualSymbolName, size);
                                    for (u64 i = 0; i < size; ++i) {
                                        globalState.memory.writeMemoryNoExcept(symbol.address + i, data);
                                    }
                                    globalState.memory.setPermission(symbol.address, size, Permission{ true, true, false });
                                }
                                break;

                            case Ast::Directive::Name::zero:
                                {
                                    u32 size = std::stoull(directive.arguments[0]);
                                    Symbol& symbol = globalState.symbolTable.addSymbol(actualSymbolName, size);
                                    for (u64 i = 0; i < size; ++i) {
                                        globalState.memory.writeMemoryNoExcept(symbol.address + i, 0u);
                                    }
                                    globalState.memory.setPermission(symbol.address, size, Permission{ true, true, false });
                                }
                                break;

                            case Ast::Directive::Name::byte:
                                {
                                    u32 size = directive.arguments.size();
                                    Symbol symbol;
                                    if (globalState.symbolTable.hasSymbol(actualSymbolName)) {
                                        symbol = globalState.symbolTable.extendSymbol(actualSymbolName, size);
                                    }
                                    else {
                                        symbol = globalState.symbolTable.addSymbol(actualSymbolName, size);
                                    }
                                    for (u32 i = 0; i < size; ++i) {
                                        u8 value = static_cast<u8>(Parser::textToNumber(directive.arguments[i]));
                                        globalState.memory.writeMemoryNoExcept(symbol.address + i, value);
                                    }
                                    globalState.memory.setPermission(symbol.address, size, permission);
                                }
                                break;

                            case Ast::Directive::Name::quad:
                                {
                                    u32 size = directive.arguments.size() * 8;
                                    Symbol symbol;
                                    if (globalState.symbolTable.hasSymbol(actualSymbolName)) {
                                        symbol = globalState.symbolTable.extendSymbol(actualSymbolName, size);
                                    }
                                    else {
                                        symbol = globalState.symbolTable.addSymbol(actualSymbolName, size);
                                    }
                                    for (u32 i = 0; i < directive.arguments.size(); ++i) {
                                        auto& text = directive.arguments[i];
                                        u64 value;
                                        if (Parser::isNumber(text) || Parser::isHexNumber(text)) {
                                            value = Parser::textToNumber(directive.arguments[i]);
                                        }
                                        else {
                                            value = globalState.symbolTable.findSymbol(text).address; // ToDO fix with meoemrey nnode
                                        }
                                        globalState.memory.writeMemoryNoExcept(symbol.address + i * 8, value);
                                    }
                                    globalState.memory.setPermission(symbol.address, size, permission);
                                }
                                break;

                            default:
                                break;
                        }
                        break;
                    }

                case 2:
                    {
                        if (section.name != "text") {
                            LOG_ERROR("Instructions can only be in the .text section");
                        }

                        Ast::Instruction instruction = std::get<Ast::Instruction>(item);
                        instructionMap[instructionID] = instruction;
                        Symbol symbol;
                        if (globalState.symbolTable.hasSymbol(actualSymbolName)) {
                            symbol = globalState.symbolTable.extendSymbol(actualSymbolName, 8);
                        }
                        else {
                            symbol = globalState.symbolTable.addSymbol(actualSymbolName, 8);
                        }
                        globalState.memory.writeMemoryNoExcept(symbol.address, instructionID);
                        globalState.memory.setPermission(symbol.address, 8, permission);
                        ++instructionID;
                        break;
                    }
                case 3:
                    {
                        // SymbolAssignment
                        const Ast::SymbolAssignment& symbolAssignment = std::get<Ast::SymbolAssignment>(item);
                        const std::vector<Token>& tokens = symbolAssignment.expression.tokens;
                        if (tokens[0].type == Token::Type::Dot && tokens[1].type == Token::Type::Dash) {
                            globalState.symbolImmediates.push_back(SymbolImmediate{ symbolAssignment.name, globalState.symbolTable.symbols[tokens[2].lexeme].size });
                        }
                        break;
                    }
            }
        }
    }

    // Execution
    LOG_DEBUG("Linking completed. Starting execution...");

    // init RIP
    u64& instructionPointer = globalState.cpu.rip;
    instructionPointer = globalState.symbolTable.findSymbol("_start").address;

    // init RSP
    globalState.cpu.rsp = UINT64_MAX;

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() / 1'000'000.;
    LOG_DEBUG("Linking completed in {} ms.", duration);

    startTime = std::chrono::high_resolution_clock::now();

    u64 counter = 0;
    while (true) {
        instructionID = 0;
        globalState.memory.readMemory(instructionPointer, instructionID);
        if (globalState.memory.getBytePermission(instructionPointer).execute == false) {
            LOG_ERROR("Execute access violation at address 0x{:016x}", instructionPointer);
        }
        counter++;
        Ast::Instruction instruction = instructionMap[instructionID];
        u32 shouldExit = Mnemonics::instructionDefinitions[instruction.mnemonic.mnemonicName].implementation(globalState, instruction);
        LOG_DEBUG("Executed instruction '{}' at RIP=0x{:016x}", instruction.mnemonic.mnemonicName, instructionPointer);
        if (shouldExit != 0) {
            endTime = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() / 1'000'000.;
            LOG_INFO("Run completed in {} ms. ({} Instructions)", duration, counter);
            return 0;
        }
    }
}

} // namespace Interpreter