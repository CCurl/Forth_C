Yes, I know ... ANOTHER Forth implementation. This one creates a "bare metal" Forth virtual machine.

The goal of this project is to create an implementation of a virtual "bare metal" Forth machine. To 
that end, the VM has less than 50 instructions, which make up the VM's opcodes and the Forth system's 
primitives. See "Defines.h" for that list of built-in functionality. The run() function in "CForth2.cpp"
is the implementation of the virtual CPU loop.

To get the initial code image into the VM, I created a simple, very Forth-like assembler. It generates an 
in-memory image of the VM's desired initial state from the "source.txt" file. It also disassembles the VM 
into a file named "dis.txt". A casual inspection of the disassembly will show that the entire system is 
just a Forth dictionary.

Currently, this implementation has the code at the beginning of the memory space, and the dictionary at the 
end, and they grow towards each other. The data and return stacks are after the dictionary.

When the VM boots, its "BIOS" start up code reads and parses the "dis.txt" file to load the initial image of 
itself, and then starts executing at address 0, which is just a JUMP to the "main" word.

The CForth2 project is the implementation of the Forth VM.
The CFComp project is the implementation of the simple Forth VM image assembler.

The VM is written in C. The assembler is written in C++.
