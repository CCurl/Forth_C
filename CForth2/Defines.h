#pragma once

// The VM's instruction set

#define RESET      0
#define PUSH       1
#define FETCH      2
#define STORE      3
#define SWAP       4
#define DROP       5
#define DUP        6
#define ROT        7
#define JMP        8
#define JMPZ       9
#define JMPNZ     10
#define CALL      11
#define RET       12
#define CPUSH     14
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
#define TUCK      27
#define FOPEN     28	// ( name mode -- fp status ) - mode: 0 = read, 1 = write
#define FREAD     29	// ( addr num fp -- count ) - fp == 0 means STDIN
#define FREADLINE 30	// ( addr fp -- count )
#define FWRITE    31	// ( addr num fp -- ) - fp == 0 means STDIN
#define FCLOSE    32	// ( fp -- )
#define BYE    255

typedef unsigned char BYTE;

typedef struct {
	int next, XT;
	BYTE flags;
	BYTE len;
	char name[30];
} DICT_T;

#define DSTACK_SZ 1024
#define RSTACK_SZ 1024

#define ADDR_SP     8
#define ADDR_RSP   12
#define ADDR_HERE  16
#define ADDR_LAST  20
#define ADDR_STATE 24
#define ADDR_BASE  28
