#include "stdafx.h"
#include <atlstr.h>
#include "CFCompiler.h"

#include "..\CFComp-Shared.inc"

typedef struct {
	CELL prev, next, XT;
	BYTE flags;
	BYTE len;
	char name[30];
} DICT_T;

extern OPCODE_T opcodes[];

CCFCompiler::CCFCompiler()
{
}


CCFCompiler::~CCFCompiler()
{
}

void CCFCompiler::Compile(LPCTSTR m_source, LPCTSTR m_output)
{
	SP = 0;
	HERE = 0;
	LAST = HERE;
	STATE = 0;
	line_no = 0;
	memset(the_memory, 0x00, sizeof(the_memory));
	SetAt(LAST, 0);

	CW2A source(m_source);
	CW2A output(m_output);
	FILE *fp_in = NULL;
	fopen_s(&fp_in, source, "rt");

	char buf[128];
	if (fp_in)
	{
		while (fgets(buf, sizeof(buf), fp_in) == buf)
		{
			++line_no;
			CString line(buf);
			line.Trim();
			Parse(line);
		}
		fclose(fp_in);
	}

	CELL start_here = FindWord(CString(_T("main")));
	if (start_here <= 0)
	{
		start_here = GetAt(LAST + 4);
	}

	SetAt(0, JMP);
	SetAt(1, start_here);
	the_memory[ADDR_CELL] = CELL_SZ;
	SetAt(ADDR_LAST, LAST);
	SetAt(ADDR_HERE, HERE);

	FILE *fp_out = NULL;
	fopen_s(&fp_out, output, "wt");
	if (fp_out)
	{
		Dis(fp_out);
		fclose(fp_out);
	}

	//fopen_s(&fp_out, output, "wt");
	//if (fp_out)
	//{
	//	for (int i = 0; i < HERE; i++)
	//	{
	//		fprintf(fp_out, "%08lx %02x\n", i, the_memory[i]);
	//	}

	//	for (int i = LAST; i < MEM_SZ; i++)
	//	{
	//		fprintf(fp_out, "%08lx %02x\n", i, the_memory[i]);
	//	}
	//	fclose(fp_out);
	//	fp_out = NULL;
	//}

}

bool CCFCompiler::ExecuteOpcode(BYTE opcode)
{
	CELL arg1, arg2, arg3;
	switch (opcode)
	{
	case ADD:
		arg2 = Pop();
		arg1 = Pop();
		arg3 = arg1+arg2;
		Push(arg3);
		return true;
	}
	return false;
}

BYTE CCFCompiler::FindAsm(CString& word)
{
	for (int i = 0; opcodes[i].opcode != 0; i++)
	{
		if (opcodes[i].asm_instr == word)
		{
			return opcodes[i].opcode;
		}
	}
	return 0;
}

BYTE CCFCompiler::FindForthPrim(CString& word)
{
	for (int i = 0; opcodes[i].opcode != 0; i++)
	{
		if (opcodes[i].forth_prim == word)
		{
			return opcodes[i].opcode;
		}
	}
	return 0;
}

CELL CCFCompiler::FindWord(CString& word)
{
	CELL cur = LAST;
	CW2A wd(word);
	do
	{
		DICT_T *dp = (DICT_T *)(&the_memory[cur]);
		if (strcmp(wd, dp->name) == 0)
		{
			return dp->XT;
		}
		cur = dp->prev;
	} while (cur > 0);

	return 0;
}

OPCODE_T *CCFCompiler::FindOpcode(BYTE opcode)
{
	for (int i = 0; opcodes[i].opcode != 0; i++)
	{
		if (opcodes[i].opcode == opcode)
		{
			return &opcodes[i];
		}
	}
	return NULL;
}

BOOL IsNumeric(char *w)
{
	// One leading minus sign is OK
	if (*w == '-')
	{
		w++;
	}

	while (*w)
	{
		if ((*w < '0') || (*w > '9'))
			return 0;
		w++;
	}
	return 1;
}

BOOL CCFCompiler::MakeNumber(CString& word, CELL& the_num)
{
	CW2A w(word);
	if (IsNumeric(w))
	{
		the_num = (CELL)atol(w);
		return 1;
	}
	return 0;
}

void CCFCompiler::DefineWord(CString& word, BYTE flags)
{
	CELL new_LAST = HERE;

	DICT_T *dp_LAST = (DICT_T *)(&the_memory[LAST]);
	DICT_T *dp_NEW = (DICT_T *)(&the_memory[new_LAST]);

	// PREV
	if (new_LAST != LAST)
	{
		Comma(LAST);
		dp_LAST->next = new_LAST;
	}
	else
	{
		FIRST = new_LAST;
		Comma(0);
	}
	//dp_NEW->next = 0;
	//dp_NEW->flags = flags;
	//dp_NEW->len = word.GetLength();

	// NEXT
	Comma(0);

	// XT - set this later
	Comma(0);

	// FLAGS
	CComma(flags);

	// LEN
	CComma(word.GetLength());

	// WORD
	CW2A wd(word);
	char *cp = wd;
	while (*cp)
	{
		CComma(*cp++);
	}

	// NULL
	CComma(NULL);

	// Fill in XT
	dp_NEW->XT = HERE;
	LAST = new_LAST;
}

void CCFCompiler::SetAt(CELL loc, CELL num)
{
	*(CELL *)(&the_memory[loc]) = num;
}

void CCFCompiler::Comma(CELL num)
{
	if (0 <= HERE) // && (HERE < LAST))
	{
		SetAt(HERE, num);
		HERE += CELL_SZ;
	}
	else
	{

	}
}

void CCFCompiler::CComma(BYTE num)
{
	if (0 <= HERE)	// < LAST)
	{
		the_memory[HERE] = num;
		HERE += 1;
	}
	else
	{

	}
}

void CCFCompiler::GetWord(CString& line, CString& word)
{
	word.Empty();
	line.TrimLeft();
	int pos = line.Find(_T(" "));
	if (pos >= 0)
	{
		word = line.Left(pos);
		line = line.Mid(pos + 1);
	}
	else
	{
		word = line;
		line.Empty();
	}
}

void CCFCompiler::Parse(CString& line)
{
	CString source = line;
	CString parsed;
	line.Replace(_T("\t"), _T(" "));
	CString word;
	while (line.GetLength())
	{
		GetWord(line, word);
		if (parsed.GetLength() > 0)
		{
			parsed.AppendChar(' ');
		}
		parsed.AppendFormat(_T("%s"), word);

		if (word == _T("\\"))
		{
			return;
		}

		if (word == ".ORG")
		{
			GetWord(line, word);
			CELL addr = 0;
			if (MakeNumber(word, addr))
			{
				HERE = addr;
				LAST = HERE;
				SetAt(LAST, 0);		// PREV = 0 means end of dictionary
			}
			continue;
		}

		//if (word == ".CELL")
		//{
		//	GetWord(line, word);
		//	CELL cell_sz = 2;
		//	if (MakeNumber(word, cell_sz))
		//	{
		//		the_memory[ADDR_CELL] = (BYTE)cell_sz;
		//	}
		//	continue;
		//}

		if ((STATE < 0) || (STATE > 2))
		{
			printf("STATE (%d) is messed up!\n", STATE);
			STATE = 0;
		}

		if (word == _T(":"))
		{
			STATE = 1;
			GetWord(line, word);
			parsed.AppendFormat(_T(" %s"), word);
			DefineWord(word, 0);
			CComma(DICTP);
			Comma(LAST);
			continue;
		}

		if (word == _T("IMMEDIATE"))
		{
			DICT_T *dp = (DICT_T *)&the_memory[LAST];
			dp->flags = 0x01;
			continue;
		}

		if (word == _T("<asm>"))
		{
			Push(STATE);
			STATE = 2;
			continue;
		}

		if (word == _T("</asm>"))
		{
			STATE = Pop();
			continue;
		}

		if (word == _T(".HERE"))
		{
			Push(HERE);
			if (STATE == 1)
			{
				CComma(LITERAL);
				Comma(Pop());
			}
			continue;
		}

		if (word == _T(".LITERAL"))
		{
			CComma(LITERAL);
			Comma(Pop());
			continue;
		}

		if (word == _T(".CLITERAL"))
		{
			CComma(CLITERAL);
			CComma((BYTE)Pop());
			continue;
		}

		if (word == _T(".COMMA"))
		{
			Comma(Pop());
			continue;
		}

		//if (word == _T(":!"))
		//{
		//	STATE = 1;
		//	GetWord(line, word);
		//	parsed.AppendFormat(_T(" %s"), word);
		//	DefineWord(word, 1);
		//	CComma(DICTP);
		//	Comma(LAST);
		//	continue;
		//}

		if (word == ";")
		{
			STATE = 0;
			CComma(RET);
			continue;
		}
		
		// These words are only for : definitions
		if (STATE == 1)
		{
			if (word == "IF")
			{
				CComma(JMPZ);
				Push(HERE);
				Comma(0);
				continue;
			}

			if (word == "ELSE")
			{
				CELL tmp = Pop();
				CComma(JMP);
				Push(HERE);
				Comma(0);
				SetAt(tmp, HERE);
				continue;
			}

			if (word == "THEN")
			{
				CELL tmp = Pop();
				SetAt(tmp, HERE);
				continue;
			}

			if (word == "BEGIN")
			{
				Push(HERE);
				continue;
			}

			if (word == "AGAIN")
			{
				CComma(JMP);
				Comma(Pop());
				continue;
			}

			if (word == "WHILE")
			{
				CComma(JMPNZ);
				Comma(Pop());
				continue;
			}

			if (word == "UNTIL")
			{
				CComma(JMPZ);
				Comma(Pop());
				continue;
			}

			if (word == "S\"")
			{
				CComma(SLITERAL);
				int cur_here = HERE++, count = 0;
				bool done = false;
				while ((!done) && (line.GetLength() > 0))
				{
					char ch = (char)line[0];
					line = line.Mid(1);
					parsed.AppendChar((TCHAR)ch);
					if (ch == '\"')
					{
						done = true;
					}
					else
					{
						the_memory[HERE++] = (BYTE)ch;
						count++;
					}
				}
				the_memory[HERE++] = (BYTE)NULL;
				the_memory[cur_here] = (BYTE)count;
				continue;
			}
		}

		BYTE opcode = 0;

		opcode = FindAsm(word);
		//if ((STATE == 0) || (STATE == 2))
		if (0 < opcode)
		{
			if (STATE == 0)
			{
				if (! ExecuteOpcode(opcode))
				{
					printf("%s: unsupported opcode %d\n", source, opcode);
				}
			}
			else 
			{
				CComma(opcode);
			}
			continue;
		}

		opcode = FindForthPrim(word);
		if (0 < opcode)
		{
			if (STATE == 1)
			{
				CComma(opcode);
			}
			else
			{
				if (!ExecuteOpcode(opcode))
				{
					printf("%s: unsupported opcode %d\n", source, opcode);
				}
			}
			continue;
		}

		CELL XT = FindWord(word);
		if (0 < XT)
		{
			if (STATE == 1)
			{
				CComma(CALL);
				Comma(XT);
			}
			else
			{
				printf("%s: cannot execute '%s'!\n", source, word);
			}
			continue;
		}

		CELL num = 0;
		if (MakeNumber(word, num))
		{
			if (STATE == 0)
			{
				Push(num);
				continue;
			}
			else if (STATE == 1)
			{
				if ((0 <= num) && (num <= 255))
				{
					CComma(CLITERAL);
					CComma((BYTE)num);
				}
				else
				{
					CComma(LITERAL);
					Comma(num);
				}
			}
			else
			{
				Push(num);
				continue;
			}
			continue;
		}

		printf("%s: '%s'??\n", source, word);
	}
}

void CCFCompiler::Dis(FILE *fp)
{
	CELL PC = 0;
	while (PC < FIRST)
	{
		PC = Dis1(PC, fp);
	}
	fprintf(fp, "\n%0*x ; The dictionary starts here ...\n", CELL_SZ*2, FIRST);

	PC = FIRST;
	do
	{
		PC = DisDict(PC, fp);
	} while (PC != 0);
}

CELL CCFCompiler::GetAt(CELL loc)
{
	return *(CELL *)(&the_memory[loc]);
}

void DisOut(FILE *fp, CString& line, CString& desc, int wid = 24)
{
	if (!desc.IsEmpty())
	{
		while (line.GetLength() < wid)
		{
			line.AppendChar(' ');
		}

		line.AppendFormat(_T("; %s"), desc);
	}

	CW2A out(line);
	fprintf(fp, "%s\n", out);
}

void CCFCompiler::GetWordName(CELL loc, CString& name)
{
	if ((0 <= loc) && (loc < MEM_SZ))
	{
		DICT_T *dp = (DICT_T *)&the_memory[loc];
		name.Format(_T("%S"), dp->name);
	}
	else
	{
		name = _T("??");
	}
}

void CCFCompiler::DisRange(CString& line, CELL loc, CELL num)
{
	while (num-- > 0)
	{
		line.AppendFormat(_T(" %02x"), the_memory[loc++]);
	}
}

CELL CCFCompiler::Dis1(CELL PC, FILE *fp)
{
	int CELL_WD = CELL_SZ * 2;
	CString line, desc = _T("(data)");
	BYTE op = the_memory[PC++];
	line.Format(_T("%0*x %02x"), CELL_WD, PC - 1, op);
	int line_len = 32;
	if (op == JMPZ)
	{
		DisRange(line, PC, CELL_SZ);
		CELL arg = GetAt(PC);
		PC += CELL_SZ;
		desc.Format(_T("JMPZ %0*x"), CELL_WD, arg);
	}

	else if (op == JMP)
	{
		DisRange(line, PC, CELL_SZ);
		CELL arg = GetAt(PC);
		desc.Format(_T("JMP %0*x"), CELL_WD, arg);
		PC += CELL_SZ;
	}

	else if (op == JMPNZ)
	{
		CELL arg = GetAt(PC);
		DisRange(line, PC, CELL_SZ);
		desc.Format(_T("JMPNZ %0*x"), CELL_WD, arg);
		PC += CELL_SZ;
	}

	else if (op == DICTP)
	{
		CELL arg = GetAt(PC);
		DisRange(line, PC, CELL_SZ);
		desc.Format(_T("DICTP %0*x"), CELL_WD, arg);
		CString tmp;
		GetWordName(arg, tmp);
		if (tmp.Compare(_T("??")) != 0)
		{
			CString tmp;
			tmp.Format(_T("\n%s"), line);
			line = tmp;
			++line_len;
		}
		desc.AppendFormat(_T(" - %s"), tmp);
		PC += CELL_SZ;
	}

	else if (op == CALL)
	{
		CELL arg = GetAt(PC);
		DisRange(line, PC, CELL_SZ);
		desc.Format(_T("CALL %0*x"), CELL_WD, arg);
		CString tmp;
		if ((0 <= arg) && (arg < LAST) && (the_memory[arg] == DICTP))
		{
			arg = GetAt(arg + 1);
			GetWordName(arg, tmp);
		}
		else
		{
			tmp = _T("(unnamed)");
		}
		desc.AppendFormat(_T(" - %s"), tmp);
		PC += CELL_SZ;
	}

	else if (op == LITERAL)
	{
		CELL arg = GetAt(PC);
		DisRange(line, PC, CELL_SZ);
		desc.Format(_T("LITERAL %0*x"), CELL_WD, arg);
		PC += CELL_SZ;
	}

	else if (op == CLITERAL)
	{
		BYTE arg = the_memory[PC++];
		line.AppendFormat(_T(" %02x"), arg);
		desc.Format(_T("CLITERAL %02x"), arg);
	}

	else if (op == SLITERAL)
	{
		desc.Format(_T("SLITERAL - "));
		BYTE arg = the_memory[PC++];		// Skip count byte
		line.AppendFormat(_T(" %02x"), arg);
		desc.AppendFormat(_T("(%d), "), (int)arg);
		arg = the_memory[PC++];
		while (arg)
		{
			line.AppendFormat(_T(" %02x"), arg);
			desc.AppendChar((TCHAR)arg);
			arg = the_memory[PC++];
		}
		line.Append(_T(" 00 "));
	}

	else
	{
		OPCODE_T *op_p = FindOpcode(op);
		if (op_p)
		{
			desc = op_p->asm_instr;
		}

	}

	DisOut(fp, line, desc, line_len);
	return PC;
}

CELL CCFCompiler::DisDict(CELL PC, FILE *fp)
{
	CString line, desc;
	int CELL_WD = CELL_SZ * 2;

	DICT_T *dp = (DICT_T *)&the_memory[PC];
	//if (dp->next == 0)
	//{
	//	line.Format(_T("\n%0*x"), CELL_WD, PC, dp->next);
	//	DisRange(line, PC, CELL_SZ);
	//	desc.Format(_T("End"));
	//	DisOut(fp, line, desc, 33);
	//	return 0;
	//}

	line.Format(_T("\n%0*x"), CELL_WD, PC);
	DisRange(line, PC, CELL_SZ*2);
	desc.Format(_T("Prev=%0*x, Next=%0*x, %S"), CELL_WD, dp->prev, CELL_WD, dp->next, dp->name);
	DisOut(fp, line, desc, 33);

	PC += CELL_SZ;
	PC += CELL_SZ;
	line.Format(_T("%0*x"), CELL_WD, PC);
	DisRange(line, PC, CELL_SZ+1);
	desc.Format(_T("XT=%0*x, flags=%02x"), CELL_WD, dp->XT, dp->flags);
	DisOut(fp, line, desc, 32);

	PC += CELL_SZ + 1;
	line.Format(_T("%0*x"), CELL_WD, PC);
	DisRange(line, PC, dp->len + 2);
	desc.Empty();
	DisOut(fp, line, desc, 32);

	PC = dp->XT;
	CELL stop_here = dp->next ? dp->next : HERE;
	while (PC < stop_here)
	{
		PC = Dis1(PC, fp);
	}

	return dp->next;
}

