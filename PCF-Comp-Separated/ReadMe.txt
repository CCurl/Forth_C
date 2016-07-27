========================================================================
    CONSOLE APPLICATION : PCF-Comp-Separated Project Overview
========================================================================

This is the compiler for my Forth system's boot loader.

In this implementation, the code and data is intermingled, and start at the beginning of the memory space. The dictionary starts at the top of the space, and grows downward.

A dictionary header looks like this:

typedef struct {
	CELL next, XT;
	BYTE flags;
	BYTE len;
	char name[30];
} DICT_T;

flags is a bit field:
#define IS_IMMEDIATE 0x01
#define IS_INLINE    0x02
#define IS_OPCODE    0x04
