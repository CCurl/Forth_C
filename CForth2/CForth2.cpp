// CForth2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
#include <malloc.h>
#include "Defines.h"
#include <stdio.h>

// ------------------------------------------------------------------------------------------
// The VM
// ------------------------------------------------------------------------------------------

#define ONE_KB (1024)
#define ONE_MB (ONE_KB * ONE_KB)
#define MEM_SZ 8*ONE_KB

typedef unsigned char BYTE;
BYTE *the_mem = NULL;
int IP = 0;
int *RSP = NULL; // the return stack pointer
int *DSP = NULL; // the data stack pointer

int RSP_ADDR = MEM_SZ - RSTACK_SZ;
int DSP_ADDR = RSP_ADDR- DSTACK_SZ;

#define GETAT(loc) *(int *)(&the_mem[loc])
#define SETAT(loc, val) *(int *)(&the_mem[loc]) = val

#define GETTOS() (*DSP)
#define SETTOS(val) (*DSP) = (val)

#define push(val) *(++DSP) = (int)(val)
#define pop() *(DSP--)

#define rpush(val) *(++RSP) = (int)(val)
#define rpop() *(RSP--)

// ------------------------------------------------------------------------------------------
// Where all the work is done
// ------------------------------------------------------------------------------------------
int run()
{
	int arg1, arg2, arg3;

	DSP = (int *)&the_mem[DSP_ADDR];
	RSP = (int *)&the_mem[RSP_ADDR];

	while (true)
	{
		BYTE opcode = the_mem[IP++];
		switch (opcode)
		{
		case PUSH:
			arg1 = GETAT(IP);
			IP += sizeof(int);
			push(arg1);
			break;

		case CPUSH:
			arg1 = the_mem[IP++];
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
			arg1 = pop();
			arg2 = pop();
			push(arg1);
			push(arg2);
			break;

		case DROP:
			pop();
			break;

		case DUP:
			arg1 = GETTOS();
			push(arg1);
			break;

		case ROT:
			arg1 = pop();
			arg2 = pop();
			arg3 = pop();
			push(arg2);
			push(arg1);
			push(arg3);
			break;

		case OVER:
			arg1 = pop();
			arg2 = pop();
			push(arg2);
			push(arg1);
			push(arg2);
			break;

		case TUCK:
			arg1 = pop();
			arg2 = pop();
			push(arg1);
			push(arg2);
			push(arg1);
			break;

		case JMP:
			IP = GETAT(IP);
			break;

		case JMPZ:
			if (pop() == 0)
			{
				IP = GETAT(IP);
			}
			else
			{
				IP += sizeof(int);
			}
			break;

		case JMPNZ:
			arg1 = pop();
			if (arg1 != 0)
			{
				IP = GETAT(IP);
			}
			else
			{
				IP += sizeof(int);
			}
			break;

		case CALL:
			arg1 = GETAT(IP);
			IP += sizeof(int);
			rpush(IP);
			IP = arg1;
			break;

		case RET:
			IP = rpop();
			break;

		case CFETCH:
			arg1 = GETTOS();
			SETTOS(the_mem[arg1]);
			break;

		case CSTORE:
			arg1 = pop();
			arg2 = pop();
			the_mem[arg1] = (BYTE)arg2;
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
			arg1 = GETAT(IP);
			IP += sizeof(int);
			break;

		case EMIT:
			arg1 = pop();
			putchar(arg1);
			break;

		case FOPEN:			// ( name mode -- fp status ) - mode: 0 = read, 1 = write
			arg2 = pop();
			arg1 = pop();
			{
				char *fileName = (char *)&the_mem[arg1 + 1];
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
				BYTE *pBuf = (BYTE *)&the_mem[arg1 + 1];
				int num = fread_s(pBuf, arg2, sizeof(BYTE), arg2, arg3 == 0 ? stdin : (FILE *)arg3);
				push(num);
			}
			break;

		case FREADLINE:			// ( addr num fp -- count ) - fp == 0 means STDIN
			arg3 = pop();
			arg2 = pop();
			arg1 = pop();
			{
				char *pBuf = (char *)&the_mem[arg1 + 1];
				FILE *fp = arg3 ? (FILE *)arg3 : stdin;
				if (fgets(pBuf, arg2, fp) != pBuf)
				{
					*pBuf = NULL;
				}
				arg2 = strlen(pBuf);
				// Strip off any trailing newline
				if ((arg2 > 0) && (pBuf[arg2-1] == '\n'))
				{
					pBuf[--arg2] = NULL;
				}
				*(--pBuf) = arg2;
				push(arg2);
			}
			break;

		case FWRITE:			// ( addr num fp -- count ) - fp == 0 means STDIN
			arg3 = pop();
			arg2 = pop();
			arg1 = pop();
			{
				BYTE *pBuf = (BYTE *)&the_mem[arg1 + 1];
				int num = fwrite(pBuf, sizeof(BYTE), arg2, arg3 == 0 ? stdin : (FILE *)arg3);
				push(num);
			}
			break;

		case FCLOSE:
			arg1 = pop();
			if (arg1 != 0)
				fclose((FILE *)arg1);
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
			arg1 = pop();
			push(arg1+1);
			break;

		case BYE:
			return pop();

		case RESET:
		default:
			DSP = (int *)&the_mem[DSP_ADDR];
			RSP = (int *)&the_mem[RSP_ADDR];
			IP = 0;
		}
	}
	return 0;
}

// ------------------------------------------------------------------------------------------
// Initialize the VM
// ------------------------------------------------------------------------------------------
char *GetNextNum(char *cp, int& val)
{
	val = 0;
	while ((0 < *cp) && (*cp <= ' '))
	{
		cp++;
	}

	while (*cp)
	{
		char c = *cp;

		if (('0' <= c) && (c <= '9'))
		{
			val = (val << 4) + (c - '0');
		}
		else if (('A' <= c) && (c <= 'F'))
		{
			val = (val << 4) + 10 + (c - 'A');
		}
		else if (('a' <= c) && (c <= 'f'))
		{
			val = (val << 4) + 10 + (c - 'a');
		}
		else 
		{
			//if (c == ';')
			//{
			//	*cp = NULL;
			//}
			// else 
			if (c > ' ')
			{
				*cp = NULL;
			}
			return cp;
		}
		cp++;
	}
	return cp;
}

// ------------------------------------------------------------------------------------------
int bios_init()
{
	IP = 0;

	char buf[128], *fn = ".\\dis.txt";
	FILE *fp = NULL;
#ifdef _DEBUG
	fn = "..\\CFComp\\dis.txt";
#endif
	fopen_s(&fp, fn, "rt");
	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp) == buf)
		{
			buf[strlen(buf - 1)] = (char)NULL;
			char *cp = GetNextNum(buf, IP);
			// printf("%s ", cp);
			while ((*cp) && (IP >= 0))
			{
				int num;
				cp = GetNextNum(cp, num);
				the_mem[IP++] = num;
			}
		}
		fclose(fp);
		IP = 0;
	}
	else
	{
		printf("Error opening '%s'!", fn);
		IP = 1;
	}
	return IP;
}

// ------------------------------------------------------------------------------------------
// The simplest possible main function
// ------------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	the_mem = (BYTE *)malloc(MEM_SZ);
	memset(the_mem, NULL, (MEM_SZ*sizeof(BYTE)));

	int initOK = bios_init();
	if (initOK != 0)
	{
		return initOK;
	}

	return run();
}
