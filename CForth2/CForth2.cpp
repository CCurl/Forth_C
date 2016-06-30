// CForth2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
#include <malloc.h>
#include "Defines.h"

// ------------------------------------------------------------------------------------------
// The VM
// ------------------------------------------------------------------------------------------

#define ONE_KB (1024)
#define ONE_MB (ONE_KB * ONE_KB)
#define MEM_SZ 8*ONE_KB

typedef unsigned char BYTE;
BYTE *the_mem = NULL;
int IP = 0;
int RSP = MEM_SZ - (1 * ONE_KB);
int SP  = MEM_SZ - (2 * ONE_KB);


#define GETAT(loc) *(int *)(&the_mem[loc])
#define SETAT(loc, val) *(int *)(&the_mem[loc]) = val

#define GETTOS() GETAT(SP)
#define SETTOS(val) SETAT(SP, val)

int push(int val)
{
	SP += sizeof(int);
	SETAT(SP, val);
	return val;
}

int pop()
{
	int ret = GETAT(SP);
	SP -= sizeof(int);
	return ret;
}

int rpush(int val)
{
	RSP += sizeof(int);
	SETAT(RSP, val);
	return val;
}

int rpop()
{
	int ret = GETAT(RSP);
	RSP -= sizeof(int);
	return ret;
}

void run()
{
	int arg1, arg2, arg3;

	IP = 0;
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

		case GETLINE:
			arg1 = pop();
			{
				char *cp = (char *)&the_mem[arg1+1];
				fgets(cp, 250, stdin);
				cp[strlen(cp) - 1] = NULL;
				*(cp-1) = strlen(cp);
				push(arg1);
			}
			break;

		case BYE:
			fputs("BYE", stdout);
			getchar();
			return;

		default:
			return;
		}
	}
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

void init_vm()
{
	IP = 0;

	char buf[128];
	FILE *fp = NULL;
	fopen_s(&fp, "dis.txt", "rt");
	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp) == buf)
		{
			buf[strlen(buf - 1)] = (char)NULL;
			char *cp = GetNextNum(buf, IP);
			while ((*cp) && (IP >= 0))
			{
				int num;
				cp = GetNextNum(cp, num);
				the_mem[IP++] = num;
			}
		}
		fclose(fp);
	}
	
	RSP = MEM_SZ - ONE_KB;
	SP = RSP - ONE_KB;
}

// ------------------------------------------------------------------------------------------
// The simplest possible main function
// ------------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	the_mem = (BYTE *)malloc(MEM_SZ);
	memset(the_mem, NULL, (MEM_SZ*sizeof(BYTE)));

	init_vm();
	run();
	return 0;
}
