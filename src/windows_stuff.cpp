// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Windows.h>

#include "windows_stuff.h"

u32 Win_GetConsoleCP() {
    return GetConsoleCP();
}

void Win_SetConsoleCP(u32 codePageID) {
    SetConsoleCP(codePageID);
}

void Win_SetConsoleOutputCP(u32 codePageID) {
    SetConsoleOutputCP(codePageID);
}
