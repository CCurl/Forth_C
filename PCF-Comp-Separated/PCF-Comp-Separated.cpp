// PCF-Comp-Integrated.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlstr.h>
#include "..\PCForth\Defines.h"
#include "CFCompiler.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CCFCompiler comp;
	comp.Compile(_T("..\\source.txt"), _T("..\\dis.txt"));
	return 0;
}

