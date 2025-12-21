AsmCube
=======

AsmCube is an experimental assembler source code interpreter that can execute AT&T style x86_64 assembly source and also
emulate some Linux syscalls.
It is currently still in the early stages of development, but can already execute a few things.
AsmCube was primarily created as a learning and experimentation platform for myself.

Current Status
--------------

Working features:

- GPRs including subregisters
- Emulated memory
- Emulated stack
- RIP relative addressing
- Memory operands disp(base, index, scale)
- Linux syscalls (read, write, open, exit)
- Instructions (lea, xor, add, sub, mov, push, pop, call, ret, hlt, leave, syscall)
- Data sections (data, rodata, bss, text)

Partially working features:

- Only 64-bit register operands are currently supported

Planned features:

- More syscalls
- More instructions
- Performance improvements
- Subsections
- Conditional jumps and loops


Motivation
----------

I wanted to learn assembler, and what better way to do that than to build a complete interpreter
that that forces you to understand it in depth and also supports me in that I don't need Linux to run it,
don't have to compile the code, and can build specific debugging tools for myself.

Goal
----

For me personally, it would be a major milestone if I could run simple programs compiled from C (freestanding) with AsmCube.

Contributing
------------

There is still a lot to do at this stage. If larger contributions are planned,
it would be good to discuss this briefly beforehand. Contributions are very welcome.
Issues can be created, please with as many details and as specific as possible.

License
-------

`GPL-3.0 license <https://github.com/Fire-Cube/AsmCube/blob/main/LICENSE>`_