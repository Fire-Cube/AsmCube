#include <iostream>

#include "parser/parser.h"
#include "registers.h"
#include "memory.h"
#include "interpreter.h"
#include "syscalls.h"
#include "mnemonics.h"

u64 resolveMemory(const Memory& memory, GlobalState& globalState) {
    u64 displacement = 0;
    u64 base = 0;
    u64 index = 0;
    u64 scale = 1;

    if (memory.disp.has_value()) {
        if (std::holds_alternative<s64>(*memory.disp)) {
            displacement = std::get<s64>(*memory.disp);
        }
        else if (std::holds_alternative<Label>(*memory.disp)) {
            displacement = globalState.symbolTable.findSymbol(std::get<Label>(*memory.disp).name).address;
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
            case Scale::One:
                scale = 1;
                break;

            case Scale::Two:
                scale = 2;
                break;

            case Scale::Four:
                scale = 4;
                break;

            case Scale::Eight:
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

std::string getOperandSize(const Operand& left, const Operand& right, const CPU& cpu, const std::string& sizeSuffix) {
    if (std::holds_alternative<Immediate>(right)) {
        LOG_ERROR("Destination cannot be an immediate");
    }
    if (std::holds_alternative<Register>(right)) {
        auto& dstReg = std::get<Register>(right);
        auto dstSize = getRegisterSize(cpu, dstReg.name.substr(1));
        if (std::holds_alternative<Register>(left)) {
            auto& srcReg = std::get<Register>(left);
            auto srcSize = getRegisterSize(cpu, srcReg.name.substr(1));
            if (dstSize != srcSize) {
                LOG_ERROR("Destination register size is different than source register size");
            }
        }
        if (!sizeSuffix.empty() && sizeSuffix != dstSize) {
            LOG_ERROR("Size suffix does not match destination register size");
        }
        return dstSize;
    }
    if (std::holds_alternative<Memory>(right)) {
        if (std::holds_alternative<Register>(left)) {
            auto& reg = std::get<Register>(left);
            return getRegisterSize(cpu, reg.name.substr(1));
        }
        if (std::holds_alternative<Memory>(left)) {
            LOG_ERROR("Memory can not be in both src and dst operands");
        }
        if (std::holds_alternative<Immediate>(left)) {
            if (!sizeSuffix.empty()) {
                return sizeSuffix;
            }
            LOG_ERROR("Size suffix is required when moving immediate to memory");
        }

    }
    LOG_ERROR("Unable to determine operand size");

}

u64 readOperand(const Operand& operand, std::string& targetSize, GlobalState& globalState) {
    switch (operand.index()) {
        case 0:
            {
                // Register
                auto& reg = std::get<Register>(operand);
                return *globalState.cpu.reg64[reg.name.substr(1)];
                // ToDo: handle other register sizes
            }

        case 1:
            {
                // Immediate
                auto& immediate = std::get<Immediate>(operand);
                if (immediate.integer.has_value()) {
                    return immediate.integer.value();
                }
                if (immediate.symbol.has_value()) {
                    for (const auto& symbolImmediate : globalState.symbolImmediates) {
                        if (symbolImmediate.name == immediate.symbol.value()) {
                            return symbolImmediate.value;
                        }
                    }
                    for (const auto& [symbolName, symbol] : globalState.symbolTable.symbols) {
                        if (symbolName == immediate.symbol.value()) {
                            return symbol.address;
                        }
                    }
                    LOG_ERROR("Unknown symbol immediate {}", immediate.symbol.value());
                }
                break;
            }

        case 2:
            {
                // Memory
                auto& memoryOperand = std::get<Memory>(operand);
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
}

void writeOperand(const Operand& operand, const u64 value, GlobalState& globalState) {
    switch (operand.index()) {
        case 0:
            {
                // Register
                auto& reg = std::get<Register>(operand);
                *globalState.cpu.reg64[reg.name.substr(1)] = value;
                break;
            }

        case 2:
            {
                // Memory
                auto& memoryOperand = std::get<Memory>(operand);
                u64 address = resolveMemory(memoryOperand, globalState);
                switch (getRegisterSize(globalState.cpu,memoryOperand.base.value().name).c_str()[0]) {
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

int run(Ast& ast) {
    GlobalState globalState{};
    std::unordered_map<u64, Instruction> instructionMap{};
    u64 instructionID = 0;

    // Linking
    LOG_DEBUG("Start linking...");

    Interpreter::Permission permission{};
    for (const Section& section : ast) {
        if (section.name == "rodata" || section.name.rfind("rodata.") == 0) {
            permission = Interpreter::Permission{ true, false, false };
        }
        else if (section.name == "data" || section.name.rfind("data.") == 0) {
            permission = Interpreter::Permission{ true, true, false };
        }
        else if (section.name == "bss" || section.name.rfind("bss.") == 0) {
            permission = Interpreter::Permission{ true, true, false };
        }
        else if (section.name == "text" || section.name.rfind("text.") == 0) {
            permission = Interpreter::Permission{ true, false, true };
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
                          actualSymbolName = std::get<Label>(item).name;
                          break;
                      }

                  case 1:
                      {
                          // Directive
                          const Directive& directive = std::get<Directive>(item);
                          switch (directive.name) {
                              case Directive::Name::ascii:
                                  {
                                      auto buffer = decodeAscii(directive.arguments[0]);
                                      Symbol& symbol = globalState.symbolTable.addSymbol(actualSymbolName,buffer.size());
                                      for (u64 i = 0; i < buffer.size(); ++i) {
                                          globalState.memory.writeMemoryNoExcept(symbol.address + i, buffer[i]);
                                      }
                                      globalState.memory.setPermission(symbol.address, buffer.size(), permission);
                                  }
                                  break;

                              case Directive::Name::skip:
                                  {
                                      u32 size = std::stoull(directive.arguments[0]);
                                      Symbol& symbol = globalState.symbolTable.addSymbol(actualSymbolName, size);
                                      for (u64 i = 0; i < size; ++i) {
                                          globalState.memory.writeMemoryNoExcept(symbol.address + i, 0u);
                                      }
                                      globalState.memory.setPermission(symbol.address, size, Interpreter::Permission{ true, true, false });
                                  }
                                  break;

                              case Directive::Name::byte:
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
                                          u8 value = static_cast<u8>(std::stoull(directive.arguments[i]));
                                          globalState.memory.writeMemoryNoExcept(symbol.address + i, value);
                                      }
                                      globalState.memory.setPermission(symbol.address, size, permission);
                                  }
                                  break;

                              case Directive::Name::quad:
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
                                          u64 value = static_cast<u64>(std::stoull(directive.arguments[i]));
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

                          Instruction instruction = std::get<Instruction>(item);
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
                          const SymbolAssignment& symbolAssignment = std::get<SymbolAssignment>(item);
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

    // init Stack
    globalState.memory.setPermission(UINT64_MAX - 8_MiB, 8_MiB, { true, true, false });

    while (true) {
        instructionID = 0;
        globalState.memory.readMemory(instructionPointer, instructionID);
        if (globalState.memory.getBytePermission(instructionPointer).execute == false) {
            LOG_ERROR("Execute access violation at address 0x{:016x}", instructionPointer);
        }
        Instruction instruction = instructionMap[instructionID];
        u32 shouldExit = instructionDefinitions[instruction.mnemonic.mnemonicName].implementation(globalState, instruction);
        if (shouldExit != 0) {
            return 0;
        }
    }

    return 0;
}