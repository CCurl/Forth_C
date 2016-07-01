#include "stdafx.h"
#include "CFCompiler.h"

OPCODE_T opcodes[] = {
	{ _T("RESET"), RESET, _T("RESET") }
	, { _T("PUSH"), PUSH, _T("") }
	, { _T("CPUSH"), CPUSH, _T("") }
	, { _T("FETCH"), FETCH, _T("@") }
	, { _T("STORE"), STORE, _T("!") }
	, { _T("SWAP"), SWAP, _T("SWAP") }
	, { _T("DROP"), DROP, _T("DROP") }
	, { _T("DUP"), DUP, _T("DUP") }
	, { _T("ROT"), ROT, _T("ROT") }
	, { _T("OVER"), OVER, _T("OVER") }
	, { _T("TUCK"), TUCK, _T("TUCK") }
	, { _T("JMP"), JMP, _T("JMP") }
	, { _T("JMPZ"), JMPZ, _T("JMPZ") }
	, { _T("JMPNZ"), JMPNZ, _T("JMPNZ") }
	, { _T("CALL"), CALL, _T("") }
	, { _T("RET"), RET, _T("LEAVE") }
	, { _T("CFETCH"), CFETCH, _T("C@") }
	, { _T("CSTORE"), CSTORE, _T("C!") }
	, { _T("ADD"), ADD, _T("+") }
	, { _T("SUB"), SUB, _T("-") }
	, { _T("MUL"), MUL, _T("*") }
	, { _T("DIV"), DIV, _T("/") }
	, { _T("LT"), LT, _T("<") }
	, { _T("EQ"), EQ, _T("=") }
	, { _T("GT"), GT, _T(">") }
	, { _T("DICTP"), DICTP, _T("DICTP") }
	, { _T("EMIT"), EMIT, _T("EMIT") }
	, { _T("FOPEN"), FOPEN, _T("FOPEN") }
	, { _T("FREAD"), FREAD, _T("FREAD") }
	, { _T("FREADLINE"), FREADLINE, _T("FREADLINE") }
	, { _T("FWRITE"), FWRITE, _T("FWRITE") }
	, { _T("FCLOSE"), FCLOSE, _T("FCLOSE") }
	, { _T("DTOR"), DTOR, _T(">R") }
	, { _T("RFETCH"), RFETCH, _T("R@") }
	, { _T("RTOD"), RTOD, _T("R>") }
	, { _T("ONEPLUS"), ONEPLUS, _T("1+") }
	, { _T("BYE"), BYE, _T("BYE") }
	, { _T(""), -1, _T("") }
};

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
	LAST = MEM_SZ - (DSTACK_SZ + RSTACK_SZ) - sizeof(int);
	STATE = 0;
	line_no = 0;
	memset(the_memory, 0x00, sizeof(the_memory));
	SetAt(LAST, 0);
	SetAt(ADDR_SP, LAST  + sizeof(int));
	SetAt(ADDR_RSP, GetAt(ADDR_SP) + DSTACK_SZ);

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

	int start_here = FindWord(CString(_T("main")));
	if (start_here <= 0)
	{
		start_here = GetAt(LAST + 4);
	}

	SetAt(0, JMP);
	SetAt(1, start_here);
	SetAt(ADDR_LAST, LAST);
	SetAt(ADDR_HERE, HERE);
	SetAt(ADDR_BASE, 10);
	SetAt(ADDR_STATE, 0);

	FILE *fp_out = NULL;
	fopen_s(&fp_out, "dis.txt", "wt");
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

int CCFCompiler::FindAsm(CString& word)
{
	for (int i = 0; opcodes[i].opcode != -1; i++)
	{
		if (opcodes[i].asm_instr == word)
		{
			return opcodes[i].opcode;
		}
	}
	return -1;
}

int CCFCompiler::FindForthPrim(CString& word)
{
	for (int i = 0; opcodes[i].opcode != -1; i++)
	{
		if (opcodes[i].forth_prim == word)
		{
			return opcodes[i].opcode;
		}
	}
	return -1;
}

int CCFCompiler::FindWord(CString& word)
{
	CW2A wd(word);
	DICT_T *dp = (DICT_T *)(&the_memory[LAST]);
	while (dp->next > 0)
	{
		if (strcmp(wd, dp->name) == 0)
		{
			return dp->XT;
		}
		dp = (DICT_T *)(&the_memory[dp->next]);
	}

	return -1;
}

OPCODE_T *CCFCompiler::FindOpcode(int opcode)
{
	for (int i = 0; opcodes[i].opcode != -1; i++)
	{
		if (opcodes[i].opcode == opcode)
		{
			return &opcodes[i];
		}
	}
	return NULL;
}

int IsNumeric(char *w)
{
	while (*w)
	{
		if ((*w < '0') || (*w > '9'))
			return 0;
		w++;
	}
	return 1;
}

int CCFCompiler::MakeNumber(CString& word, int& the_num)
{
	CW2A w(word);
	if (IsNumeric(w))
	{
		the_num = atol(w);
		return 1;
	}
	return 0;
}

void CCFCompiler::DefineWord(CString& word, int flags)
{
	int tmp = LAST;
	LAST -= (11 + word.GetLength());

	DICT_T *dp = (DICT_T *)(&the_memory[LAST]);
	dp->next = tmp;
	dp->XT = HERE;
	dp->flags = flags;
	dp->len = word.GetLength();

	tmp = 0;
	CW2A wd(word);
	char *cp = wd;
	while (*cp)
	{
		dp->name[tmp++] = *(cp++);
	}
	dp->name[tmp++] = NULL;
}

void CCFCompiler::SetAt(int loc, int num)
{
	*(int *)(&the_memory[loc]) = num;
}

void CCFCompiler::Comma(int num)
{
	if ((HERE < LAST) && (HERE < (MEM_SZ - 4)))
	{
		SetAt(HERE, num);
		HERE += 4;
	}
	else
	{

	}
}

void CCFCompiler::CComma(int num)
{
	if (HERE < LAST)
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
			int addr = 0;
			if (MakeNumber(word, addr))
			{
				HERE = addr;
			}
			continue;
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

		if (word == _T("<asm>"))
		{
			Push(STATE);
			STATE = 2;
		}

		if (word == _T("</asm>"))
		{
			STATE = Pop();
		}

		if (word == _T(":!"))
		{
			STATE = 1;
			GetWord(line, word);
			parsed.AppendFormat(_T(" %s"), word);
			DefineWord(word, 1);
			CComma(DICTP);
			Comma(LAST);
			continue;
		}

		if (word == ";")
		{
			STATE = 0;
			CComma(RET);
			continue;
		}

		if (word == "IF")
		{
			CComma(JMPZ);
			Push(HERE);
			Comma(-1);
			continue;
		}

		if (word == "ELSE")
		{
			int tmp = Pop();
			CComma(JMP);
			Push(HERE);
			Comma(-1);
			SetAt(tmp, HERE);
			continue;
		}

		if (word == "THEN")
		{
			int tmp = Pop();
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

		int opcode = 0;

		if ((STATE == 0) || (STATE == 2))
		{
			opcode = FindAsm(word);
			if (opcode >= 0)
			{
				CComma(opcode);
				continue;
			}
		}

		opcode = FindForthPrim(word);
		if (opcode >= 0)
		{
			CComma(opcode);
			continue;
		}

		int XT = FindWord(word);
		if (XT > 0)
		{
			CComma(CALL);
			Comma(XT);
			continue;
		}

		int num = 0;
		if (MakeNumber(word, num))
		{
			if (num < 256)
			{
				if (STATE == 1)
					CComma(CPUSH);
				CComma(num);
			}
			else
			{
				if (STATE == 1)
					CComma(PUSH);
				Comma(num);
			}
			continue;
		}

		// Error here?
	}
}

void CCFCompiler::Dis(FILE *fp)
{
	int PC = 0;
	while (PC < HERE)
	{
		PC = Dis1(PC, fp);
	}

	fprintf(fp, "\n%08lx ; The dictionary starts here ...\n", LAST);

	PC = LAST;
	while (PC >= LAST)
	{
		PC = DisDict(PC, fp);
	}
}

int CCFCompiler::GetAt(int loc)
{
	return *(int *)(&the_memory[loc]);
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

void CCFCompiler::GetWordName(int loc, CString& name)
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

void CCFCompiler::DisRange(CString& line, int loc, int num)
{
	while (num-- > 0)
	{
		line.AppendFormat(_T(" %02x"), the_memory[loc++]);
	}
}

int CCFCompiler::Dis1(int PC, FILE *fp)
{
	CString line, desc = _T("(data)");
	int op = the_memory[PC++];
	line.Format(_T("%08lx %02x"), PC - 1, op);
	int line_len = 32;
	if (op == JMPZ)
	{
		DisRange(line, PC, 4);
		int arg = GetAt(PC);
		PC += 4;
		desc.Format(_T("JMPZ %08lx"), arg);
	}

	else if (op == JMP)
	{
		int arg = GetAt(PC);
		DisRange(line, PC, 4);
		desc.Format(_T("JMP %08lx"), arg);
		PC += 4;
	}

	else if (op == JMPNZ)
	{
		int arg = GetAt(PC);
		DisRange(line, PC, 4);
		desc.Format(_T("JMPNZ %08lx"), arg);
		PC += 4;
	}

	else if (op == DICTP)
	{
		int arg = GetAt(PC);
		DisRange(line, PC, 4);
		desc.Format(_T("DICTP %08lx"), arg);
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
		PC += 4;
	}

	else if (op == CALL)
	{
		int arg = GetAt(PC);
		DisRange(line, PC, 4);
		desc.Format(_T("CALL %08lx"), arg);
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
		PC += 4;
	}

	else if (op == PUSH)
	{
		int arg = GetAt(PC);
		DisRange(line, PC, 4);
		desc.Format(_T("PUSH %08lx"), arg);
		PC += 4;
	}

	else if (op == CPUSH)
	{
		int arg = the_memory[PC++];
		line.AppendFormat(_T(" %02x"), arg);
		desc.Format(_T("CPUSH %02x"), arg);
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

int CCFCompiler::DisDict(int PC, FILE *fp)
{
	CString line, desc;

	DICT_T *dp = (DICT_T *)&the_memory[PC];
	if (dp->next == 0)
	{
		line.Format(_T("\n%08lx"), PC, dp->next);
		DisRange(line, PC, 4);
		desc.Format(_T("End"));
		DisOut(fp, line, desc, 33);
		return 0;
	}

	line.Format(_T("\n%08lx"), PC);
	DisRange(line, PC, 4);
	desc.Format(_T("Next=%08lx, %S"), dp->next, dp->name);
	DisOut(fp, line, desc, 33);

	PC += 4;
	line.Format(_T("%08lx"), PC);
	DisRange(line, PC, 5);
	desc.Format(_T("XT=%08lx, flags=%02x"), dp->XT, dp->flags);
	DisOut(fp, line, desc, 32);

	PC += 5;
	line.Format(_T("%08lx"), PC);
	DisRange(line, PC, dp->len + 2);
	desc.Empty();
	DisOut(fp, line, desc, 32);

	return dp->next;
}

