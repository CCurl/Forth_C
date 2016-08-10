// PCF-Comp-Integrated.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlstr.h>
#include "CFCompiler.h"

extern BYTE *the_memory;

void GetArg(TCHAR *prefix, int argc, TCHAR *argv[], CString& out)
{
	CString match = prefix;
	int len = match.GetLength();
	for (int i = 1; i < argc; i++)
	{
		CString arg = argv[i];
		if (arg.Left(len).CompareNoCase(match) == 0)
		{
			out = arg.Mid(len, 999);
			break;
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	the_memory = (BYTE *)malloc(MEM_SZ);
	memset(the_memory, NULL, MEM_SZ);

	CString iFn, oFn, bFn;

	iFn = "source-sep.txt";
	oFn = "dis-sep.txt";
	bFn = "vm_image.bin";

	GetArg(_T("-i:"), argc, argv, iFn);
	GetArg(_T("-o:"), argc, argv, oFn);
	GetArg(_T("-b:"), argc, argv, bFn);

	CCFCompiler comp;
	comp.Compile(iFn, oFn, bFn);
	free(the_memory);
	return 0;
}
