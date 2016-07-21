// PCF-Comp-Integrated.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlstr.h>
#include "..\PCForth\Defines.h"
#include "CFCompiler.h"

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
	CString iFn, oFn;

	iFn = "..\\source-sep.txt";
	oFn = "..\\dis-sep.txt";

	GetArg(_T("-i:"), argc, argv, iFn);
	GetArg(_T("-o:"), argc, argv, oFn);

	CCFCompiler comp;
	comp.Compile(iFn, oFn);
	return 0;
}
