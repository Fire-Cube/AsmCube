#pragma once

#include <string>
#include <unordered_map>

#include "types.h"
#include "logging.h"
#include "registers.h"
#include "memory.h"
#include "parser/parser.h"

struct Symbol {
    u64 address;
    u64 size;
};

class SymbolTable {
    private:
        u64 addressPointer = 0;

    public:
        std::unordered_map<std::string, Symbol> symbols;

        Symbol& addSymbol(std::string symbolName, u64 size) {
            if (symbols.find(symbolName) != symbols.end())
                LOG_ERROR("Symbol '{}' already defined!", symbolName);

            symbols[symbolName] = (Symbol{ addressPointer, size });
            addressPointer += size;
            return symbols[symbolName];
        }
        Symbol& findSymbol(std::string symbolName) {
            if (symbols.find(symbolName) != symbols.end()) {
                return symbols[symbolName];
            }
            LOG_ERROR("Symbol '{}' not found!", symbolName);
        }

};

struct SymbolImmediate {
    std::string name;
    u64 value;
};

struct GlobalState {
    CPU cpu{};
    Interpreter::Memory memory{};
    SymbolTable symbolTable{};
    std::vector<SymbolImmediate> symbolImmediates{};
};

u64 resolveMemory(const Memory& memory, GlobalState& globalState);
std::string getOperandSize(const Operand& left, const Operand& right, const CPU& cpu, const std::string& sizeSuffix);
u64 readOperand(const Operand& operand, std::string& targetSize, GlobalState& globalState);
void writeOperand(const Operand& operand, u64 value, GlobalState& globalState);
int run(Ast& ast);