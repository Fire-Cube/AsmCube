// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <magic_enum/magic_enum.hpp>

#include "types.h"
#include "logging.h"

namespace Interpreter::Instructions::Helper
{

inline bool evaluateCondCodes(Ast::CondCode condCode, const GlobalState& globalState) {
    switch (condCode) {
        case Ast::CondCode::overflow:
            return globalState.cpu.of;
        case Ast::CondCode::notOverflow:
            return !globalState.cpu.of;
        case Ast::CondCode::sign:
            return globalState.cpu.sf;
        case Ast::CondCode::notSign:
            return !globalState.cpu.sf;
        case Ast::CondCode::equal:
        case Ast::CondCode::zero:
            return globalState.cpu.zf;
        case Ast::CondCode::notEqual:
        case Ast::CondCode::notZero:
            return !globalState.cpu.zf;
        case Ast::CondCode::below:
        case Ast::CondCode::notAboveOrEqual:
        case Ast::CondCode::carry:
            return globalState.cpu.cf;
        case Ast::CondCode::notBelow:
        case Ast::CondCode::aboveOrEqual:
        case Ast::CondCode::notCarry:
            return !globalState.cpu.cf;
        case Ast::CondCode::belowOrEqual:
        case Ast::CondCode::notAbove:
            return globalState.cpu.cf ||globalState.cpu.zf;
        case Ast::CondCode::above:
        case Ast::CondCode::notBelowOrEqual:
            return !globalState.cpu.cf && !globalState.cpu.zf;
        case Ast::CondCode::less:
        case Ast::CondCode::notGreaterOrEqual:
            return globalState.cpu.sf != globalState.cpu.of;
        case Ast::CondCode::greaterOrEqual:
        case Ast::CondCode::notLess:
            return globalState.cpu.sf == globalState.cpu.of;
        case Ast::CondCode::lessOrEqual:
        case Ast::CondCode::notGreater:
            return globalState.cpu.zf || (globalState.cpu.sf != globalState.cpu.of);
        case Ast::CondCode::greater:
        case Ast::CondCode::notLessOrEqual:
            return !globalState.cpu.zf && (globalState.cpu.sf == globalState.cpu.of);
        case Ast::CondCode::parity:
        case Ast::CondCode::parityEven:
            return globalState.cpu.pf;
        case Ast::CondCode::notParity:
        case Ast::CondCode::parityOdd:
            return !globalState.cpu.pf;
        default:
            LOG_ERROR("Unknown condition code {}", magic_enum::enum_name(condCode));
    }
}

inline void calculateFlagsSFZFPF(GlobalState& globalState, u64 result, u64 width) {
    // SF
    u64 sign = 1ULL << (width - 1);
    u64 signedResult = result & sign;
    globalState.cpu.sf = (signedResult != 0);

    // ZF
    globalState.cpu.zf = (result == 0);

    // PF
    globalState.cpu.pf = (std::popcount(static_cast<u8>(result)) % 2) == 0;
}

} // namespace Interpreter::Instructions::Helper
