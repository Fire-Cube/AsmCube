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
- Emulated stack
- RIP relative addressing
- Memory operands disp(base, index, scale)
- Emulated Linux syscalls (read, write, open, exit)
- Instructions (lea, xor, add, sub, mov, push, pop, call, ret, hlt, leave, syscall, Jcc)
- Data sections (data, rodata, bss, text)
- Code Labels

Planned features:

- More syscalls
- More instructions
- Performance improvements

Motivation
----------

I wanted to learn assembler, and what better way to do that than to build a complete interpreter / emulator
that that forces you to understand it in depth and also supports me in that I don't need Linux to run it,
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

License
-------

`GPL-3.0 license <https://github.com/Fire-Cube/AsmCube/blob/main/LICENSE>`_