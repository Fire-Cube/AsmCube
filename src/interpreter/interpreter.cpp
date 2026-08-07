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
            if (memory.base.has_value() && memory.base->name == "rip") {
                displacement -= globalState.cpu.rip + 8;
            }
        }
    }

    if (memory.base.has_value()) {
        if (memory.base.value().name == "rip") {
            base = globalState.cpu.rip + 8;
        }
        else {
            base = *globalState.cpu.reg64.at(memory.base->index);
        }
    }

    if (memory.index.has_value()) {
        index = *globalState.cpu.reg64.at(memory.index.value().index);
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


Ast::Width getOperandSize(const Ast::Operand& left, const std::optional<Ast::Width> suffix) {
    if (std::holds_alternative<Ast::Register>(left)) {
        const auto& reg = std::get<Ast::Register>(left);
        if (suffix.has_value() && suffix.value() != reg.width) {
            LOG_ERROR("Size suffix does not match register size");
        }
        return reg.width;
    }
    if (std::holds_alternative<Ast::Memory>(left)) {
        if (suffix.has_value()) {
            return *suffix;
        }
        LOG_ERROR("Size suffix is required when operand is memory");
    }
    LOG_ERROR("Cannot determine operand size");
}

Ast::Width getOperandSize(const Ast::Operand& left, const Ast::Operand& right, const std::optional<Ast::Width> suffix) {
    if (std::holds_alternative<Ast::Immediate>(right)) {
        LOG_ERROR("Destination cannot be an immediate");
    }
    if (std::holds_alternative<Ast::Register>(right)) {
        auto& dstReg = std::get<Ast::Register>(right);
        auto dstSize = dstReg.width;
        if (std::holds_alternative<Ast::Register>(left)) {
            auto& srcReg = std::get<Ast::Register>(left);
            auto srcSize = srcReg.width;
            if (dstSize != srcSize) {
                LOG_ERROR("Destination register size is different than source register size");
            }
        }
        if (suffix.has_value() && *suffix != dstSize) {
            LOG_ERROR("Size suffix '{}' does not match destination register size '{}'", magic_enum::enum_name(*suffix), magic_enum::enum_name(dstSize));
        }
        return dstSize;
    }
    if (std::holds_alternative<Ast::Memory>(right)) {
        if (std::holds_alternative<Ast::Register>(left)) {
            auto& reg = std::get<Ast::Register>(left);
            return reg.width;
        }
        if (std::holds_alternative<Ast::Memory>(left)) {
            LOG_ERROR("Memory can not be in both src and dst operands");
        }
        if (std::holds_alternative<Ast::Immediate>(left)) {
            if (suffix.has_value()) {
                return *suffix;
            }
            LOG_ERROR("Size suffix is required when moving immediate to memory");
        }

    }
    LOG_ERROR("Unable to determine operand size");
}

u64 readOperand(const Ast::Operand& operand,  Ast::Width targetSize, GlobalState& globalState) {
    switch (static_cast<Ast::OperandType>(operand.index())) {
        case Ast::OperandType::Register:
            {
                const auto& reg = std::get<Ast::Register>(operand);
                switch (reg.width) {
                    case Ast::Width::Quad:
                        return *globalState.cpu.reg64[reg.index];
                    case Ast::Width::Long:
                        return *globalState.cpu.reg32[reg.index];
                    case Ast::Width::Word:
                        return *globalState.cpu.reg16[reg.index];
                    case Ast::Width::Byte:
                        return *globalState.cpu.reg8 [reg.index];
                }
                LOG_ERROR("Invalid register width for '{}'", reg.name);
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
                switch (targetSize) {
                    case Ast::Width::Byte:
                        globalState.memory.readMemory<u8>(address, reinterpret_cast<u8&>(value));
                        break;

                    case Ast::Width::Word:
                        globalState.memory.readMemory<u16>(address, reinterpret_cast<u16&>(value));
                        break;

                    case Ast::Width::Long:
                        globalState.memory.readMemory<u32>(address, reinterpret_cast<u32&>(value));
                        break;

                    case Ast::Width::Quad:
                        globalState.memory.readMemory<u64>(address, value);
                        break;
                }
                return value;
            }
    }
    LOG_ERROR("Unhandled operand type in readOperand");
}

void writeOperand(const Ast::Operand& operand, const u64 value,  Ast::Width targetSize, GlobalState& globalState) {
    switch (static_cast<Ast::OperandType>(operand.index())) {
        case Ast::OperandType::Register:
            {
                // Register
                auto& reg = std::get<Ast::Register>(operand);

                switch (reg.width) {
                    case Ast::Width::Quad:
                        *globalState.cpu.reg64[reg.index] = value;
                        return;

                    case Ast::Width::Long:
                        *reinterpret_cast<u64*>(globalState.cpu.reg32[reg.index]) = value;
                        return;

                    case Ast::Width::Word:
                        *globalState.cpu.reg16[reg.index] = static_cast<u16>(value);
                        return;

                    case Ast::Width::Byte:
                        *globalState.cpu.reg8[reg.index] = static_cast<u8>(value);
                        return;
                }
                LOG_ERROR("Invalid register width for '{}'", reg.name);
            }

        case Ast::OperandType::Memory:
            {
                // Memory
                auto& memoryOperand = std::get<Ast::Memory>(operand);
                u64 address = resolveMemory(memoryOperand, globalState);
                switch (targetSize) {
                    case Ast::Width::Byte:
                        globalState.memory.writeMemory<u8>(address, static_cast<u8>(value));
                        break;

                    case Ast::Width::Word:
                        globalState.memory.writeMemory<u16>(address, static_cast<u16>(value));
                        break;

                    case Ast::Width::Long:
                        globalState.memory.writeMemory<u32>(address, static_cast<u32>(value));
                        break;

                    case Ast::Width::Quad:
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
    std::vector<LinkedInstruction> instructionList{};
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
                        LinkedInstruction linkedInstruction{ instruction, Mnemonics::instructionDefinitions[instruction.mnemonic.mnemonicName].implementation };
                        instructionList.push_back(linkedInstruction);
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
        LinkedInstruction& instruction = instructionList[instructionID];
        u32 shouldExit = instruction.implementation(globalState, instruction.instruction);
        LOG_DEBUG("Executed instruction '{}' at RIP=0x{:016x}", instruction.instruction.mnemonic.mnemonicName, instructionPointer);
        if (shouldExit != 0) {
            endTime = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count() / 1'000'000.;
            LOG_INFO("Run completed in {} ms. ({} Instructions)", duration, counter);
            return 0;
        }
    }
}

} // namespace Interpreter