#pragma once

// ************************************************************************************************
// The VM's instruction set
// ************************************************************************************************
#define LITERAL    1
#define FETCH      2
#define STORE      3
#define SWAP       4
#define DROP       5
#define DUP        6
#define SLITERAL   7
#define JMP        8
#define JMPZ       9
#define JMPNZ     10
#define CALL      11
#define RET       12
#define ZTYPE     13
#define CLITERAL  14
#define CFETCH    15
#define CSTORE    16
#define ADD       17
#define SUB       18
#define MUL       19
#define DIV       20
#define LT        21
#define EQ        22
#define GT        23
#define DICTP     24
#define EMIT      25
#define OVER      26
#define COMPARE   27	// ( addr1 addr2 -- bool )
#define FOPEN     28	// ( name mode -- fp status ) - mode: 0 = read, 1 = write
#define FREAD     29	// ( addr num fp -- count ) - fp == 0 means STDIN
#define FREADLINE 30	// ( addr fp -- count )
#define FWRITE    31	// ( addr num fp -- ) - fp == 0 means STDIN
#define FCLOSE    32	// ( fp -- )
#define DTOR      33	// >R (Data To Return)
#define RFETCH    34	// R@
#define RTOD      35	// R> (Return To Data)
#define ONEPLUS   36
#define PICK      37
#define DEPTH     38
#define GETCH     39
#define LSHIFT    40
#define RSHIFT    41
#define AND       42
#define OR		  43
#define BRANCH    44
#define BRANCHZ   45
#define BRANCHNZ  46
#define BREAK    253
#define RESET    254
#define BYE      255
// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************

typedef unsigned char BYTE;
typedef long CELL;				// Use long for a 32-bit implementation, short for a 16-bit
#define CELL_SZ sizeof(CELL)

#define DSTACK_SZ CELL_SZ * 64
#define RSTACK_SZ CELL_SZ * 64

#define STACK_BUF_CELLS 2
#define STACK_BUF_SZ (STACK_BUF_CELLS * CELL_SZ)

#define ADDR_CELL   7
#define ADDR_SP     8
#define ADDR_RSP   12
#define ADDR_HERE  16
#define ADDR_LAST  20
#define ADDR_STATE 24
#define ADDR_BASE  28

#define ONE_KB (1024)
#define ONE_MB (ONE_KB * ONE_KB)
#define MEM_SZ (16*ONE_KB)

#define RSP_BASE (MEM_SZ - RSTACK_SZ)				// Start address of the return stack
#define RSP_INIT (MEM_SZ - STACK_BUF_SZ)			// Initial value of the return stack pointer

#define DSP_BASE (MEM_SZ - RSTACK_SZ - DSTACK_SZ)	// Start address of the data stack
#define DSP_INIT (DSP_BASE + STACK_BUF_SZ)			// Initial value of the data stack pointer
