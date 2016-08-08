// CForth2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include "..\Shared.h"

// ------------------------------------------------------------------------------------------
// The VM
// ------------------------------------------------------------------------------------------

typedef unsigned char BYTE;

BYTE *the_memory;
CELL PC = 0;		// The "program counter"
BYTE IR = 0;		// The "instruction register"

CELL *dsp_init = NULL;
CELL *rsp_init = NULL;
CELL arg1, arg2, arg3;

CELL *RSP = NULL; // the return stack pointer
CELL *DSP = NULL; // the data stack pointer

bool isEmbedded = false;
bool isBYE = false;


// ------------------------------------------------------------------------------------------
void init_vm()
{
	dsp_init = (CELL *)&the_memory[DSP_INIT];
	rsp_init = (CELL *)&the_memory[RSP_INIT];
	DSP = dsp_init;
	RSP = rsp_init;
	isBYE = false;
	isEmbedded = false;
	PC = 0;
}

// ------------------------------------------------------------------------------------------
// Where all the work is done
// ------------------------------------------------------------------------------------------
void cpu_step()
{
	IR = the_memory[PC++];
	switch (IR)
	{
	case LITERAL:
		arg1 = GETAT(PC);
		PC += CELL_SZ;
		push(arg1);
		break;

	case CLITERAL:
		arg1 = the_memory[PC++];
		push(arg1);
		break;

	case FETCH:
		arg1 = GETTOS();
		arg2 = GETAT(arg1);
		SETTOS(arg2);
		break;

	case STORE:
		arg1 = pop();
		arg2 = pop();
		SETAT(arg1, arg2);
		break;

	case SWAP:
		arg1 = GET2ND();
		arg2 = GETTOS();
		SET2ND(arg2);
		SETTOS(arg1);
		break;

	case DROP:
		pop();
		break;

	case DUP:
		arg1 = GETTOS();
		push(arg1);
		break;

	case OVER:
		arg1 = GET2ND();
		push(arg1);
		break;

	case PICK:
		arg1 = pop();
		arg2 = *(DSP - arg1);
		push(arg2);
		break;

	case JMP:
		PC = GETAT(PC);
		break;

	case JMPZ:
		if (pop() == 0)
		{
			PC = GETAT(PC);
		}
		else
		{
			PC += CELL_SZ;
		}
		break;

	case JMPNZ:
		arg1 = pop();
		if (arg1 != 0)
		{
			PC = GETAT(PC);
		}
		else
		{
			PC += CELL_SZ;
		}
		break;

	case BRANCH:
		arg1 = GETAT(PC);
		PC += arg1;
		break;

	case BRANCHZ:
		arg1 = GETAT(PC);
		if (pop() == 0)
		{
			arg1 = GETAT(PC);
			PC += arg1;
		}
		else
		{
			PC += CELL_SZ;
		}
		break;

	case BRANCHNZ:
		arg1 = GETAT(PC);
		if (pop() != 0)
		{
			arg1 = GETAT(PC);
			PC += arg1;
		}
		else
		{
			PC += CELL_SZ;
		}
		break;

	case CALL:
		arg1 = GETAT(PC);
		PC += CELL_SZ;
		rpush(PC);
		PC = arg1;
		break;

	case RET:
		if (RSP == rsp_init)
		{
			if (isEmbedded)
			{
				isBYE = true;
			}
			else
			{
				PC = 0;
			}
		}
		else
		{
			PC = rpop();
		}
		break;

	case COMPARE:
		arg2 = pop();
		arg1 = pop();
		{
			char *cp1 = (char *)&the_memory[arg1];
			char *cp2 = (char *)&the_memory[arg2];
			arg3 = strcmp(cp1, cp2) ? 0 : 1;
			push(arg3);
		}
		break;

	case COMPAREI:
		arg2 = pop();
		arg1 = pop();
		{
			char *cp1 = (char *)&the_memory[arg1];
			char *cp2 = (char *)&the_memory[arg2];
			arg3 = _strcmpi(cp1, cp2) ? 0 : 1;
			push(arg3);
		}
		break;

	case SLITERAL:
		arg1 = PC++;
		arg2 = the_memory[PC++];
		while (arg2)
		{
			arg2 = the_memory[PC++];
		}
		// PC++;
		push(arg1);
		break;

	case CFETCH:
		arg1 = GETTOS();
		SETTOS(the_memory[arg1]);
		break;

	case CSTORE:
		arg1 = pop();
		arg2 = pop();
		the_memory[arg1] = (BYTE)arg2;
		break;

	case ADD:
		arg1 = pop();
		arg2 = pop();
		push(arg2 + arg1);
		break;

	case SUB:
		arg1 = pop();
		arg2 = pop();
		push(arg2 - arg1);
		break;

	case MUL:
		arg1 = pop();
		arg2 = pop();
		push(arg2 * arg1);
		break;

	case DIV:
		arg1 = pop();
		arg2 = pop();
		push(arg2 / arg1);
		break;

	case LT:
		arg1 = pop();
		arg2 = pop();
		push(arg2 < arg1 ? 1 : 0);
		break;

	case EQ:
		arg1 = pop();
		arg2 = pop();
		push(arg2 == arg1 ? 1 : 0);
		break;

	case GT:
		arg1 = pop();
		arg2 = pop();
		push(arg2 > arg1 ? 1 : 0);
		break;

	case DICTP:
		// arg1 = GETAT(PC);
		PC += CELL_SZ;
		break;

	case EMIT:
		arg1 = pop();
		putchar(arg1);
		break;

	case ZTYPE:
		arg1 = pop();		// addr
		{
			char *cp = (char *)&the_memory[arg1];
			printf("%s", cp);
		}
		break;

	case FOPEN:			// ( name mode -- fp status ) - mode: 0 = read, 1 = write
		arg2 = pop();
		arg1 = pop();
		{
			char *fileName = (char *)&the_memory[arg1 + 1];
			char mode[4];
			sprintf_s(mode, sizeof(mode), "%cb", arg2 == 0 ? 'r' : 'w');
			FILE *fp = NULL;
			fopen_s(&fp, fileName, mode);
			push((int)fp);
			push(fp != NULL ? 0 : 1);
		}
		break;

	case FREAD:			// ( addr num fp -- count ) - fp == 0 means STDIN
		arg3 = pop();
		arg2 = pop();
		arg1 = pop();
		{
			BYTE *pBuf = (BYTE *)&the_memory[arg1 + 1];
			int num = fread_s(pBuf, arg2, sizeof(BYTE), arg2, arg3 == 0 ? stdin : (FILE *)arg3);
			push(num);
		}
		break;

	case FREADLINE:			// ( addr num fp -- count ) - fp == 0 means STDIN
		arg3 = pop();
		arg2 = pop();
		arg1 = pop();
		{
			char *pBuf = (char *)&the_memory[arg1 + 1];
			FILE *fp = arg3 ? (FILE *)arg3 : stdin;
			if (fgets(pBuf, arg2, fp) != pBuf)
			{
				*pBuf = NULL;
			}
			arg2 = (CELL)strlen(pBuf);
			// Strip off any trailing newline
			if ((arg2 > 0) && (pBuf[arg2 - 1] == '\n'))
			{
				pBuf[--arg2] = NULL;
			}
			*(--pBuf) = (char)arg2;
			push(arg2);
		}
		break;

	case FWRITE:			// ( addr num fp -- count ) - fp == 0 means STDIN
		arg3 = pop();
		arg2 = pop();
		arg1 = pop();
		{
			BYTE *pBuf = (BYTE *)&the_memory[arg1 + 1];
			int num = fwrite(pBuf, sizeof(BYTE), arg2, arg3 == 0 ? stdin : (FILE *)arg3);
			push(num);
		}
		break;

	case FCLOSE:
		arg1 = pop();
		if (arg1 != 0)
			fclose((FILE *)arg1);
		break;

	case GETCH:
		arg1 = _getch();
		push(arg1);
		break;

	case DTOR:
		arg1 = pop();
		rpush(arg1);
		break;

	case RFETCH:
		push(*RSP);
		break;

	case RTOD:
		arg1 = rpop();
		push(arg1);
		break;

	case ONEPLUS:
		arg1 = GETTOS();
		SETTOS(arg1 + 1);
		break;

	case DEPTH:
		arg1 = DSP - dsp_init;
		push(arg1);
		break;

	case LSHIFT:
		arg1 = pop();
		arg2 = pop();
		push(arg2 << arg1);
		break;

	case RSHIFT:
		arg1 = pop();
		arg2 = pop();
		push(arg2 >> arg1);
		break;

	case AND:
		arg1 = pop();
		arg2 = pop();
		push(arg2 & arg1);
		break;

	case OR:
		arg1 = pop();
		arg2 = pop();
		push(arg2 | arg1);
		break;

	case BREAK:
	{
		arg1 = the_memory[ADDR_HERE];
		arg2 = the_memory[ADDR_LAST];
		arg3 = arg2 - arg1;
	}
	break;

	case BYE:
		isBYE = true;
		break;

	case RESET:
	default:
		DSP = dsp_init;
		RSP = rsp_init;
		PC = 0;
		isBYE = isEmbedded;
	}
}

// ------------------------------------------------------------------------------------------
CELL cpu_loop()
{
	while (true)
	{
		cpu_step();
		if (isBYE)
		{
			return GETTOS();
		}
	}
}
