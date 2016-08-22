Yes, I know ... ANOTHER Forth implementation. This one creates a "bare metal" Forth virtual machine.

The goal of this project is to create an implementation of a virtual "bare metal" Forth machine. To 
that end, the VM has less than 50 instructions, which make up the VM's opcodes and the Forth system's 
primitives. See "Shared.h" for that list of built-in functionality. The run() function in "PCForth.cpp"
is the implementation of the virtual CPU loop.

To get the initial code image into the VM, I created a simple, Forth-like compiler/assembler. It generates 
an in-memory image of the VM's desired initial state from the -i:<fn> file. It also creates a listing file 
in the file named by -o:<fn>. A casual inspection of the disassembly will show that the entire system is 
just a Forth dictionary.

There are 2 versions of the compiler: one with the dictionary and code separated, the other with them 
integrated. They both generate images for the same VM.

The compiler that generates the separated Forth system is PCF-Comp-Separated.
The compiler that generates the integrated Forth system is PCF-Comp-Integrated.

They both take -i:<filename> and -o:<filename> for input and output arguments.

When the VM boots, its "BIOS" start up code reads and parses the -i:<fn> file to load the initial image
of itself, and then starts executing at address 0, which is just a JUMP to the "main" word. If there is
no "main", then it jumps to the last word defined.

The PCForth project is the implementation of the Forth VM.
The PCF-Comp-* projects are the implementation of the simple Forth VM image compilers.

The VM is written in C. The compilers are written in C++.
