========================================================================
    CONSOLE APPLICATION : PCForth Project Overview
========================================================================

This is the Forth virtual machine. It is as simple and bare bones as I could make it. There are three functions:

main() - calls boot_loader() and then run()
boot_loader() - loads the "OS" from the file specified by -i:<filename>
run() - the virtual CPU loop. Always starts with PC = 0.
