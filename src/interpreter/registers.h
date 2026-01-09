// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "types.h"

struct GPR {
    u64 memory; // This memory is shared with all sub-registers

    u64& rax;
    u32& eax;
    u16& ax;
    u8& ah;
    u8& al;

    GPR()
        : memory(0),
          rax(memory),
          eax(*reinterpret_cast<u32*>(std::addressof(memory))),
          ax (*reinterpret_cast<u16*>(std::addressof(memory))),
          ah (*(reinterpret_cast<u8*>(std::addressof(memory)) + 1)),
          al (*(reinterpret_cast<u8*>(std::addressof(memory))))
    {}
};

struct CPU {
    private:
        GPR _rax{};
        GPR _rbx{};
        GPR _rcx{};
        GPR _rdx{};
        GPR _rsi{};
        GPR _rdi{};
        GPR _rsp{};
        GPR _rbp{};
        GPR _r8{};
        GPR _r9{};
        GPR _r10{};
        GPR _r11{};
        GPR _r12{};
        GPR _r13{};
        GPR _r14{};
        GPR _r15{};
        GPR _rip{};

    public:
        // RAX
        u64& rax = _rax.rax;
        u32& eax = _rax.eax;
        u16& ax  = _rax.ax;
        u8&  ah  = _rax.ah;
        u8&  al  = _rax.al;

        // RBX
        u64& rbx = _rbx.rax;
        u32& ebx = _rbx.eax;
        u16& bx  = _rbx.ax;
        u8&  bh  = _rbx.ah;
        u8&  bl  = _rbx.al;

        // RCX
        u64& rcx = _rcx.rax;
        u32& ecx = _rcx.eax;
        u16& cx  = _rcx.ax;
        u8&  ch  = _rcx.ah;
        u8&  cl  = _rcx.al;

        // RDX
        u64& rdx = _rdx.rax;
        u32& edx = _rdx.eax;
        u16& dx  = _rdx.ax;
        u8&  dh  = _rdx.ah;
        u8&  dl  = _rdx.al;

        // RSI
        u64& rsi = _rsi.rax;
        u32& esi = _rsi.eax;
        u16& si  = _rsi.ax;
        u8&  sil = _rsi.al;

        // RDI
        u64& rdi = _rdi.rax;
        u32& edi = _rdi.eax;
        u16& di  = _rdi.ax;
        u8&  dil = _rdi.al;

        // RSP
        u64& rsp = _rsp.rax;
        u32& esp = _rsp.eax;
        u16& sp  = _rsp.ax;
        u8&  spl = _rsp.al;

        // RBP
        u64& rbp = _rbp.rax;
        u32& ebp = _rbp.eax;
        u16& bp  = _rbp.ax;
        u8&  bpl = _rbp.al;

        // R8
        u64& r8  = _r8.rax;
        u32& r8d = _r8.eax;
        u16& r8w = _r8.ax;
        u8&  r8b = _r8.al;

        // R9
        u64& r9  = _r9.rax;
        u32& r9d = _r9.eax;
        u16& r9w = _r9.ax;
        u8&  r9b = _r9.al;

        // R10
        u64& r10  = _r10.rax;
        u32& r10d = _r10.eax;
        u16& r10w = _r10.ax;
        u8&  r10b = _r10.al;

        // R11
        u64& r11  = _r11.rax;
        u32& r11d = _r11.eax;
        u16& r11w = _r11.ax;
        u8&  r11b = _r11.al;

        // R12
        u64& r12  = _r12.rax;
        u32& r12d = _r12.eax;
        u16& r12w = _r12.ax;
        u8&  r12b = _r12.al;

        // R13
        u64& r13  = _r13.rax;
        u32& r13d = _r13.eax;
        u16& r13w = _r13.ax;
        u8&  r13b = _r13.al;

        // R14
        u64& r14  = _r14.rax;
        u32& r14d = _r14.eax;
        u16& r14w = _r14.ax;
        u8&  r14b = _r14.al;

        // R15
        u64& r15  = _r15.rax;
        u32& r15d = _r15.eax;
        u16& r15w = _r15.ax;
        u8&  r15b = _r15.al;

        // RIP
        u64& rip  = _rip.rax;
        u32& eip  = _rip.eax;
        u16& ip   = _rip.ax;

        std::unordered_map<std::string, u64*> reg64 = {
            {"rax", &rax},
            {"rbx", &rbx},
            {"rcx", &rcx},
            {"rdx", &rdx},
            {"rsi", &rsi},
            {"rdi", &rdi},
            {"rsp", &rsp},
            {"rbp", &rbp},
            {"r8" , &r8},
            {"r9" , &r9},
            {"r10", &r10},
            {"r11", &r11},
            {"r12", &r12},
            {"r13", &r13},
            {"r14", &r14},
            {"r15", &r15},
            {"rip", &rip},
        };

        std::unordered_map<std::string, u32*> reg32 = {
            {"eax" , &eax},
            {"ebx" , &ebx},
            {"ecx" , &ecx},
            {"edx" , &edx},
            {"esi" , &esi},
            {"edi" , &edi},
            {"esp" , &esp},
            {"ebp" , &ebp},
            {"r8d" , &r8d},
            {"r9d" , &r9d},
            {"r10d", &r10d},
            {"r11d", &r11d},
            {"r12d", &r12d},
            {"r13d", &r13d},
            {"r14d", &r14d},
            {"r15d", &r15d},
            {"eip",  &eip},
        };

        std::unordered_map<std::string, u16*> reg16 = {
            {"ax"  , &ax},
            {"bx"  , &bx},
            {"cx"  , &cx},
            {"dx"  , &dx},
            {"si"  , &si},
            {"di"  , &di},
            {"sp"  , &sp},
            {"bp"  , &bp},
            {"r8w" , &r8w},
            {"r9w" , &r9w},
            {"r10w", &r10w},
            {"r11w", &r11w},
            {"r12w", &r12w},
            {"r13w", &r13w},
            {"r14w", &r14w},
            {"r15w", &r15w},
            {"ip"  , &ip},
        };

        std::unordered_map<std::string, u8*> reg8 = {
            {"ah"  , &ah},
            {"bh"  , &bh},
            {"ch"  , &ch},
            {"dh"  , &dh},
            {"al"  , &al},
            {"bl"  , &bl},
            {"cl"  , &cl},
            {"dl"  , &dl},
            {"sil" , &sil},
            {"dil" , &dil},
            {"spl" , &spl},
            {"bpl" , &bpl},
            {"r8b" , &r8b},
            {"r9b" , &r9b},
            {"r10b", &r10b},
            {"r11b", &r11b},
            {"r12b", &r12b},
            {"r13b", &r13b},
            {"r14b", &r14b},
            {"r15b", &r15b},
        };

        unsigned int cf : 1 = 0; // Carry Flag
        unsigned int pf : 1 = 0; // Parity Flag
        unsigned int af : 1 = 0; // Auxiliary Carry Flag
        unsigned int zf : 1 = 0; // Zero Flag
        unsigned int sf : 1 = 0; // Sign Flag
        unsigned int tf : 1 = 0; // Trap Flag
        unsigned int If : 1 = 0; // Interrupt Enable Flag
        unsigned int df : 1 = 0; // Direction Flag
        unsigned int of : 1 = 0; // Overflow Flag
        unsigned int iopl : 2 = 0; // I/O Privilege Level
        unsigned int nt : 1 = 0; // Nested Task Flag
        unsigned int rf : 1 = 0; // Resume Flag
        unsigned int vm : 1 = 0; // Virtual-8086 Mode
        unsigned int ac : 1 = 0; // Alignment Check / Access Control
        unsigned int vif : 1 = 0; // Virtual Interrupt Flag
        unsigned int vip : 1 = 0; // Virtual Interrupt Pending
        unsigned int id : 1 = 0; // ID Flag
};
