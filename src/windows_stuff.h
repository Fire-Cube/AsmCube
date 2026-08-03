// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "types.h"

u32 Win_GetConsoleCP();
void Win_SetConsoleCP(u32 codePageID);
void Win_SetConsoleOutputCP(u32 codePageID);