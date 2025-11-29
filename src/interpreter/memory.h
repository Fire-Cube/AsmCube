#pragma once

#include <array>
#include <bitset>
#include <unordered_map>

#include "logging.h"
#include "types.h"

namespace Interpreter
{

constexpr u32 PageSize = 4096; // 4 KiB

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
        template <std::unsigned_integral T>
        void writeMemory(const u64 address, const T& data) {
            u32 dataSize = sizeof(data);
            for (u32 i = 0; i < dataSize / sizeof(u8); ++i) {
                u32 offset = (address + i) % PageSize;
                Page& page = pages[(address + i) / PageSize];
                if (!page.permissionWrite.test(offset)) {
                   LOG_ERROR("Write access violation at address 0x{:016x}", address + i);
                }
                page.data[offset] = data >> (8 * i) & 0xFF;
                page.initialized.set(offset);
            }
        }

        template <std::unsigned_integral T>
        void writeMemoryNoExcept(const u64 address, const T& data) {
            u32 dataSize = sizeof(data);
            for (u32 i = 0; i < dataSize / sizeof(u8); ++i) {
                u32 offset = (address + i) % PageSize;
                Page& page = pages[(address + i) / PageSize];
                page.data[offset] = data >> (8 * i) & 0xFF;
                page.initialized.set(offset);
            }
        }

        template <std::unsigned_integral T>
        void readMemory(const u64 address, T& data) {
            /* data has to be zeroed before reading */

            u32 dataSize = sizeof(data);
            for (u32 i = 0; i < dataSize / sizeof(u8); ++i) {
                u32 offset = (address + i) % PageSize;
                Page& page = pages[(address + i) / PageSize];
                if (!page.permissionRead.test(offset)) {
                   LOG_ERROR("Read access violation at address 0x{:016x}", address + i);
                }
                if (!page.initialized.test(offset)) {
                   LOG_DEBUG("Reading uninitialized memory at address 0x{:016x}", address + i);
                }
                data |= static_cast<T>(page.data[offset]) << (8 * i);
            }
        }

        void setPermission(const u64 address, const u64 size, Permission permission) {
            for (u64 i = address; i < address + size; ++i) {
                u32 offset = i % PageSize;
                Page& page = pages[i / PageSize];

                if (permission.read) {
                    page.permissionRead.set(offset);
                } else {
                    page.permissionRead.reset(offset);
                }

                if (permission.write) {
                    page.permissionWrite.set(offset);
                } else {
                    page.permissionWrite.reset(offset);
                }

                if (permission.execute) {
                    page.permissionExecute.set(offset);
                } else {
                    page.permissionExecute.reset(offset);
                }
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