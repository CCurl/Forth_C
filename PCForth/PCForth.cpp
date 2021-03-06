// CForth2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <windows.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
//#include <windows.h>
#include "..\Shared.h"


extern CELL PC;
extern BYTE *the_memory;
extern void init_vm();
extern CELL cpu_loop();

// ------------------------------------------------------------------------------------------
// Initialize the VM
// ------------------------------------------------------------------------------------------
char *GetNextNum(char *cp, CELL& val)
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
bool boot_load(char *arg)
{
	int num_read = 0;
	FILE *fp = NULL;

	fopen_s(&fp, arg, "rb");
	if (!fp)
	{
		char fn[256];
		sprintf_s(fn, "..\\%s", arg);
		fopen_s(&fp, fn, "rb");
	}
	if (fp)
	{
		num_read = fread(the_memory, sizeof(BYTE), MEM_SZ, fp);
		fclose(fp);
	}
	else
	{
		printf("boot_load(): error opening '%s'!", arg);
	}

	return (num_read == MEM_SZ);
}

// ------------------------------------------------------------------------------------------
bool bios_init(char *arg)
{
	CELL addr = 0;

	char buf[128];
	FILE *fp = NULL;
	
	fopen_s(&fp, arg, "rt");
	if (!fp)
	{
		char fn[256];
		sprintf_s(fn, "..\\%s", arg);
		fopen_s(&fp, fn, "rt");
	}
	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp) == buf)
		{
			buf[strlen(buf)-1] = (char)NULL;
			char *cp = GetNextNum(buf, addr);
			while ((*cp) && (addr >= 0))
			{
				CELL num;
				cp = GetNextNum(cp, num);
				the_memory[addr++] = (BYTE)num;
			}
		}
		fclose(fp);
		return true;
	}
	else
	{
		printf("bios_init(): error opening '%s'!", arg);
	}
	return false;
}

// ------------------------------------------------------------------------------------------
// The simplest possible main function
// ------------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	//HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//DWORD dwMode = 0;
	//GetConsoleMode(hOut, &dwMode);
	//dwMode |= 0x04;
	//SetConsoleMode(hOut, dwMode);

	the_memory = (BYTE *)malloc(MEM_SZ);
	memset(the_memory, NULL, (MEM_SZ*sizeof(BYTE)));

	char arg[64];
	sprintf_s(arg, "dis-sep.txt");
	bool isBinary = false, isOK = false;

	if (argc > 1)
	{
		TCHAR *xxx = argv[1];
		if ((xxx[0] == '-') && (xxx[1] == 'i') && (xxx[2] == ':'))
		{
			xxx += 3;
			sprintf_s(arg, sizeof(arg), "%S", xxx);
		}

		else if ((xxx[0] == '-') && (xxx[1] == 'b') && (xxx[2] == ':'))
		{
			xxx += 3;
			sprintf_s(arg, sizeof(arg), "%S", xxx);
			isBinary = true;
		}
	}

	init_vm();
	if (isBinary)
	{
		isOK = boot_load(arg);
	}
	else
	{
		isOK = bios_init(arg);
	}

	if (isOK)
	{
		PC = 0;
		return cpu_loop();
	}

	return 1;
}
