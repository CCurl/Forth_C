Yes ... ANOTHER Forth implementation. This one creates a "bare metal" Forth virtual machine.

The goal of this project was to create an implementation of a virtual "bare metal" Forth machine. To that end, the VM's "CPU" knows less than 50 instructions. See Defines.h for that list of instructions.

To get the initial code image into the VM, I created a simple, very Forth-like assembler. It generates an in-memory image of the VM's desired initial state from the "source.txt" file. It also disassembles the VM into a file named "dis.txt". A casual inspection of the disassembly will show that the entire system is just a Forth dictionary.

Currently, this implementation has the code at the bottom of the memory space, and the dictionary at the top, and they grow towards each other.

When the VM boots, its "BIOS" reads the "dis.txt" file to load the initial image of itself, and then starts executing at address 0, which is just a JUMP to the "main" word.

The VM is written in C. The assembler is written in C++.
