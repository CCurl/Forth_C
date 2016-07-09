#pragma once

// The VM's instruction set

#define LITERAL    1
#define FETCH      2
#define STORE      3
#define SWAP       4
#define DROP       5
#define DUP        6
// #define ROT        7
#define JMP        8
#define JMPZ       9
#define JMPNZ     10
#define CALL      11
#define RET       12
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
// #define TUCK      27
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
#define BREAK    253
#define RESET    254
#define BYE      255

typedef short CELL;				// Use long for a 32-bit implementation, short for a 16-bit
#define CELL_SZ sizeof(CELL)
typedef unsigned char BYTE;

typedef struct {
	CELL next, XT;
	BYTE flags;
	BYTE len;
	char name[30];
} DICT_T;

#define DSTACK_SZ CELL_SZ * 64
#define RSTACK_SZ CELL_SZ * 64

#define ADDR_CELL   7
#define ADDR_SP     8
#define ADDR_RSP   12
#define ADDR_HERE  16
#define ADDR_LAST  20
#define ADDR_STATE 24
#define ADDR_BASE  28
