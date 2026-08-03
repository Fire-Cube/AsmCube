AsmCube
=======

AsmCube is an experimental x86-64 instruction-semantics emulator.
It executes decoded x86_64 instructions using an emulated CPU state and virtual memory model.
Currently, these decoded instructions are produced by an AT&T syntax assembler frontend.
It is still in the early stages of development, but can already execute a few things.
AsmCube was primarily created as a learning and experimentation platform for myself.

Current Status
--------------

Working features:

- GPRs including subregisters
- Flags register
- Emulated memory
- Memory permissions (read, write, execute) with per-byte granularity
- Detection of reads from uninitialized memory
- Emulated stack
- RIP relative addressing
- Memory operands disp(base, index, scale)
- Emulated Linux syscalls (read, write, open, exit)
- Instructions (lea, xor, and, add, sub, cmp, inc, dec, neg, test, stc, mov,
  push, pop, call, ret, jmp, Jcc, CMOVcc, hlt, leave, syscall)
- Data sections (data, rodata, bss, text)
- Code Labels
- Operand validation against instruction definitions
- Testcases with register checkpoints (YAML)
- CPU self-test
- Negative and hexadecimal number literals

Planned features:

- More syscalls
- More instructions
- Performance improvements

Motivation
----------

I wanted to learn assembler, and what better way to do that than to build a complete interpreter / emulator
that forces you to understand it in depth and also supports me in that I don't need Linux to run it,
don't have to compile the code, and can build specific debugging tools for myself.

Goal
----

For me personally, it would be a major milestone if I could run simple programs compiled from C (freestanding) with AsmCube.
(Edit): A simple "Hello World" program compiled from C can now be run with AsmCube!

Contributing
------------

There is still a lot to do at this stage. If larger contributions are planned,
it would be good to discuss this briefly beforehand. Contributions are very welcome.
Issues can be created, please with as many details and as specific as possible.

Building
--------

Requires CMake 3.30+ and a C++23 compiler.

.. code-block:: console

   cmake --preset x64-Clang-Release
   cmake --build Build/x64-Clang-Release

Available presets: ``x64-Clang-*``, ``x64-MSVC-*`` (Windows) and
``x64-GCC-*`` (Linux), each in ``Debug``, ``Release`` and ``RelWithDebInfo``.

License
-------

`GPL-3.0 license <https://github.com/Fire-Cube/AsmCube/blob/main/LICENSE>`_