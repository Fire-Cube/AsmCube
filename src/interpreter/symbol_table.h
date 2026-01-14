// SPDX-FileCopyrightText: Copyright 2026 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <unordered_map>

#include "logging.h"

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
            if (symbols.find(symbolName) != symbols.end()) {
                LOG_ERROR("Symbol '{}' already defined!", symbolName);
            }
            symbols[symbolName] = (Symbol{ addressPointer, size });
            addressPointer += size;
            return symbols[symbolName];
        }
        Symbol extendSymbol(std::string symbolName, u64 additionalSize) {
            if (symbols.find(symbolName) == symbols.end()) {
                LOG_ERROR("Symbol '{}' not defined!", symbolName);
            }

            Symbol newSymbol = symbols[symbolName];
            newSymbol.address = addressPointer;

            symbols[symbolName].size += additionalSize;
            addressPointer += additionalSize;

            return newSymbol;
        }
        bool hasSymbol(std::string symbolName) {
            return symbols.find(symbolName) != symbols.end();
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
