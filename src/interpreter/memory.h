// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <bitset>
#include <unordered_map>

#include "logging.h"
#include "types.h"

namespace Interpreter
{

constexpr u32 PageSize = 4_KiB;

struct Page {
    std::array<u8, PageSize> data {};
    std::bitset<PageSize> initialized {};
    std::bitset<PageSize> permissionRead {};
    std::bitset<PageSize> permissionWrite {};
    std::bitset<PageSize> permissionExecute {};
};

struct Permission {
    bool read = false;
    bool write = false;
    bool execute = false;
};

class Memory {
    private:
        std::unordered_map<u64, Page> pages;

    public:
        Page& getPage(const u64 address) {
            const u64 pageIndex = address / PageSize;

            auto [it, inserted] = pages.try_emplace(pageIndex);
            if (inserted && address >= UINT64_MAX - 8_MiB) {
                setPermission(pageIndex * PageSize, PageSize, Permission{ true, true, false });
            }
            return it->second;
        }

        template <std::unsigned_integral T>
        void writeMemory(const u64 address, const T& data) {
            // fast path if all data is in one page
            u32 offset = address % PageSize;
            u32 dataSize = sizeof(data);

            if (offset + dataSize <= PageSize) {
                Page& page = getPage(address);
                for (u32 i = 0; i < dataSize; ++i) {
                    if (!page.permissionWrite.test(offset + i)) {
                        LOG_ERROR("Write access violation at address 0x{:016x}", address + i);
                    }
                    page.data[offset + i] = data >> (8 * i) & 0xFF;
                    page.initialized.set(offset + i);
                }
                return;
            }

            // slow path
            for (u32 i = 0; i < dataSize / sizeof(u8); ++i) {
                u32 offset = (address + i) % PageSize;
                Page& page = getPage(address + i);

                if (!page.permissionWrite.test(offset)) {
                   LOG_ERROR("Write access violation at address 0x{:016x}", address + i);
                }
                page.data[offset] = data >> (8 * i) & 0xFF;
                page.initialized.set(offset);
            }
        }

        template <std::unsigned_integral T>
        void writeMemoryNoExcept(const u64 address, const T& data) {
            // fast path if all data is in one page
            u32 offset = address % PageSize;
            u32 dataSize = sizeof(data);

            if (offset + dataSize <= PageSize) {
                Page& page = getPage(address);
                for (u32 i = 0; i < dataSize; ++i) {
                    page.data[offset + i] = data >> (8 * i) & 0xFF;
                    page.initialized.set(offset + i);
                }
                return;
            }

            // slow path
            for (u32 i = 0; i < dataSize / sizeof(u8); ++i) {
                u32 offset = (address + i) % PageSize;
                Page& page = getPage(address + i);
                page.data[offset] = data >> (8 * i) & 0xFF;
                page.initialized.set(offset);
            }
        }

        template <std::unsigned_integral T>
        void readMemory(const u64 address, T& data) {
            // fast path if all data is in one page
            u32 offset = address % PageSize;
            u32 dataSize = sizeof(data);

            if (offset + dataSize <= PageSize) {
                Page& page = getPage(address);
                data = 0;
                for (u32 i = 0; i < dataSize; ++i) {
                    if (!page.permissionRead.test(offset + i)) {
                        LOG_ERROR("Read access violation at address 0x{:016x}", address + i);
                    }
                    if (logLevel == LogLevel::Debug && !page.initialized.test(offset + i)) {
                        LOG_DEBUG("Reading uninitialized memory at address 0x{:016x}", address + i);
                    }
                    data |= static_cast<T>(page.data[offset + i]) << (8 * i);
                }
                return;
            }

            // slow path
            data = 0;
            for (u32 i = 0; i < dataSize / sizeof(u8); ++i) {
                offset = (address + i) % PageSize;
                Page& page = getPage(address + i);
                if (!page.permissionRead.test(offset)) {
                   LOG_ERROR("Read access violation at address 0x{:016x}", address + i);
                }
                if (logLevel == LogLevel::Debug && !page.initialized.test(offset)) {
                   LOG_DEBUG("Reading uninitialized memory at address 0x{:016x}", address + i);
                }
                data |= static_cast<T>(page.data[offset]) << (8 * i);
            }
        }

        template <std::unsigned_integral T>
        void readMemoryNoExcept(const u64 address, T& data) {
            // fast path if all data is in one page
            u32 offset = address % PageSize;
            u32 dataSize = sizeof(data);

            if (offset + dataSize <= PageSize) {
                Page& page = getPage(address);
                data = 0;
                for (u32 i = 0; i < dataSize; ++i) {
                    data |= static_cast<T>(page.data[offset + i]) << (8 * i);
                }
                return;
            }

            // slow path
            data = 0;
            for (u32 i = 0; i < dataSize / sizeof(u8); ++i) {
                u32 offset = (address + i) % PageSize;
                Page& page = getPage(address + i);
                data |= static_cast<T>(page.data[offset]) << (8 * i);
            }
        }

        void setPermission(const u64 address, const u64 size, Permission permission) {
            u64 current = address;
            u64 remaining = size;
            while (remaining > 0) {
                Page& page = pages[current / PageSize];
                const u64 offset = current % PageSize;
                const u64 count = std::min(remaining, PageSize - offset);
                for (u64 n = 0; n < count; ++n) {
                    if (permission.read) {
                        page.permissionRead.set(offset + n);
                    } else {
                        page.permissionRead.reset(offset + n);
                    }

                    if (permission.write) {
                        page.permissionWrite.set(offset + n);
                    } else {
                        page.permissionWrite.reset(offset + n);
                    }

                    if (permission.execute) {
                        page.permissionExecute.set(offset + n);
                    } else {
                        page.permissionExecute.reset(offset + n);
                    }
                }
                current += count;
                remaining -= count;
            }
        }

        Permission getBytePermission(const u64 address) {
            u32 offset = address % PageSize;
            Page& page = pages[address / PageSize];

            Permission permission = {};
            permission.read = page.permissionRead.test(offset);
            permission.write = page.permissionWrite.test(offset);
            permission.execute = page.permissionExecute.test(offset);
            return permission;
        }
};

} // namespace Interpreter
