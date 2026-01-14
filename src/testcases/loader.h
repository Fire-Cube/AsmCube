// SPDX-FileCopyrightText: Copyright 2026 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <ryml.hpp>
#include <ryml_std.hpp>

#include "global_state.h"

namespace Testcases
{

void loadTest(GlobalState& globalState, const std::filesystem::path& path);

} // namespace Testcases
