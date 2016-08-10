#include "stdafx.h"
#include <atlstr.h>
#include "CFCompiler.h"

// Values for the flags bit field
#define IS_IMMEDIATE 0x01
#define IS_INLINE    0x02

#include "..\CFComp-Shared.inc"

bool isTailJmpSafe;
extern BYTE *the_memory;
extern void init_vm();
extern CELL cpu_loop();
extern void cpu_step();
extern CELL PC;
extern CELL *DSP, *RSP;
extern bool isEmbedded;
extern bool isBYE;

CELL HERE, LAST, STATE;
CELL BASE = 10;

CCFCompiler::CCFCompiler()
{
	init_vm();
	the_memory[0] = RET;
	isEmbedded = true;
}


CCFCompiler::~CCFCompiler()
{
}

void CCFCompiler::Compile(LPCTSTR m_source, LPCTSTR m_output, LPCTSTR m_binary)
{
	HERE = 0;
	LAST = DSP_BASE - CELL_SZ;
	STATE = 0;
	line_no = 0;

	SetAt(LAST, 0);
	the_memory[ADDR_CELL] = CELL_SZ;
	SetAt(ADDR_BASE, BASE);

	CW2A source(m_source);
	CW2A output(m_output);
	CW2A binary(m_binary);
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
	else
	{
		printf("error opening '%s'!", source);
		return;
	}

	DICT_T *dp = FindWord(CString(_T("main")));
	if (dp == NULL)
	{
		dp = (DICT_T *)&the_memory[LAST];
	}

	the_memory[0] = JMP;
	SetAt(1, dp->XT);

	SyncMem(true);

	FILE *fp_out = NULL;
	fopen_s(&fp_out, output, "wt");
	if (fp_out)
	{
		Dis(fp_out);
		fclose(fp_out);
	}

	fp_out = NULL;
	fopen_s(&fp_out, binary, "wb");
	if (fp_out)
	{
		fwrite(the_memory, sizeof(BYTE), MEM_SZ, fp_out);
		fclose(fp_out);
	}
}

void CCFCompiler::SyncMem(bool isSet)
{
	if (isSet)
	{
		SetAt(ADDR_LAST, LAST);
		SetAt(ADDR_HERE, HERE);
		SetAt(ADDR_BASE, BASE);
	}
	else
	{
		LAST = GetAt(ADDR_LAST);
		HERE = GetAt(ADDR_HERE);
		BASE = GetAt(ADDR_BASE);
	}
}

bool CCFCompiler::ExecuteOpcode(BYTE opcode)
{
	PC = HERE + 10;
	the_memory[PC] = opcode;

	SyncMem(true);
	cpu_step();
	SyncMem(false);

	return (PC > 0);
}

CELL CCFCompiler::ExecuteXT(CELL XT)
{
	SyncMem(true);
	PC = XT;
	isBYE = false;
	CELL ret = cpu_loop();
	SyncMem(false);

	return ret;
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

DICT_T *CCFCompiler::FindWord(CString& word)
{
	CW2A wd(word);
	DICT_T *dp = (DICT_T *)(&the_memory[LAST]);
	while (dp->next > 0)
	{
		if (strcmp(wd, dp->name) == 0)
		{
			return dp;
		}
		dp = (DICT_T *)(&the_memory[dp->next]);
	}

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

BOOL CCFCompiler::MakeNumber(CString& word, CELL& the_num)
{
	CW2A tw(word);
	bool is_neg = false;
	the_num = 0;
	char *w = tw;

	// One leading minus sign is OK
	if (*w == '-')
	{
		is_neg = true;
		w++;
	}

	while (*w)
	{
		char ch = *(w++);
		CELL digit = -1;
		if (('0' <= ch) && (ch <= '9'))
		{
			digit = ch - '0';
		}

		if (BASE == 16)
		{
			if (('A' <= ch) && (ch <= 'F'))
			{
				digit = (ch - 'A') + 10;
			}
			if (('a' <= ch) && (ch <= 'f'))
			{
				digit = (ch - 'a') + 10;
			}
		}
		if (digit < 0)
		{
			return false;
		}
		the_num = (the_num * BASE) + digit;
	}

	if (is_neg)
	{
		the_num = -the_num;
	}
	return true;
}

void CCFCompiler::DefineWord(CString& word, BYTE flags)
{
	CELL tmp = LAST;
	LAST -= ((CELL_SZ*2) + 3 + word.GetLength());

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

void CCFCompiler::SetAt(CELL loc, CELL num)
{
	*(CELL *)(&the_memory[loc]) = num;
}

void CCFCompiler::Comma(CELL num)
{
	if ((0 <= HERE) && (HERE < LAST))
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

bool CCFCompiler::IsTailJmpSafe()
{
	return isTailJmpSafe;
}

void CCFCompiler::MakeTailJmpSafe()
{
	isTailJmpSafe = true;
}

void CCFCompiler::MakeTailJmpUnSafe()
{
	isTailJmpSafe = false;
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
			}
			continue;
		}

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
			dp->flags |= IS_IMMEDIATE;
			continue;
		}

		if (word == _T("INLINE"))
		{
			DICT_T *dp = (DICT_T *)&the_memory[LAST];
			dp->flags |= IS_INLINE;
			if (IsTailJmpSafe() && (the_memory[HERE - CELL_SZ - 1] == JMP))
			{
				the_memory[HERE - CELL_SZ - 1] = CALL;
				CComma(RET);
			}
			continue;
		}

		if (word == _T("<asm>"))
		{
			STATE = 2;
			continue;
		}

		if (word == _T("</asm>"))
		{
			STATE = 1;
			continue;
		}

		if (word == _T(".HERE"))
		{
			push(HERE);
			if (STATE == 1)
			{
				CComma(LITERAL);
				Comma(pop());
			}
			continue;
		}

		if (word == _T(".CELL"))
		{
			push(CELL_SZ);
			continue;
		}

		if (word == _T(".LITERAL"))
		{
			CComma(LITERAL);
			Comma(pop());
			continue;
		}

		if (word == _T(".CLITERAL"))
		{
			CComma(CLITERAL);
			CComma((BYTE)pop());
			continue;
		}

		if (word == _T(".COMMA"))
		{
			Comma(pop());
			continue;
		}

		// These words are only for : definitions
		if (STATE == 1)
		{
			if (word == ";")
			{
				if (IsTailJmpSafe() && (the_memory[HERE - CELL_SZ - 1] == CALL))
				{
					the_memory[HERE - CELL_SZ - 1] = JMP;
				}
				else
				{
					CComma(RET);
				}
				STATE = 0;
				continue;
			}

			if (word == "IF")
			{
				CComma(BRANCHZ);
				push(HERE);
				Comma(0);
				MakeTailJmpUnSafe();
				continue;
			}

			if (word == "ELSE")
			{
				CELL tmp = pop();
				CComma(BRANCH);
				push(HERE);
				Comma(0);
				//SetAt(tmp, HERE);
				CELL offset = HERE - tmp;
				SetAt(tmp, offset);
				MakeTailJmpUnSafe();
				continue;
			}

			if (word == "THEN")
			{
				CELL tmp = pop();
				//SetAt(tmp, HERE);
				CELL offset = HERE - tmp;
				SetAt(tmp, offset);
				MakeTailJmpUnSafe();
				continue;
			}

			if (word == "BEGIN")
			{
				push(HERE);
				MakeTailJmpUnSafe();
				continue;
			}

			if (word == "AGAIN")
			{
				//CComma(JMP);
				//Comma(Pop());
				CComma(BRANCH);
				CELL tgt = pop();
				CELL offset = tgt - HERE;
				Comma(offset);
				MakeTailJmpUnSafe();
				continue;
			}

			if (word == "WHILE")
			{
				//CComma(JMPNZ);
				//Comma(Pop());
				CComma(BRANCHNZ);
				CELL tgt = pop();
				CELL offset = tgt - HERE;
				Comma(offset);
				MakeTailJmpUnSafe();
				continue;
			}

			if (word == "UNTIL")
			{
				//CComma(JMPZ);
				//Comma(Pop());
				CComma(BRANCHZ);
				CELL tgt = pop();
				CELL offset = tgt - HERE;
				Comma(offset);
				MakeTailJmpUnSafe();
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
					wprintf(_T("%s: unsupported opcode %d\n"), (LPCTSTR)source, opcode);
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
					wprintf(_T("%s: unsupported opcode %d\n"), (LPCTSTR)source, opcode);
				}
			}
			continue;
		}

		MakeTailJmpUnSafe();

		DICT_T *dp = FindWord(word);
		if (dp != NULL)
		{
			if (STATE == 1)
			{
				if (dp->flags & IS_INLINE)
				{
					// Skip the DICTP instruction
					CELL addr = dp->XT + CELL_SZ + 1;

					// Copy bytes until the first RET
					while (true)
					{
						BYTE b = the_memory[addr++];
						if (b != RET)
						{
							CComma(b);
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					CComma(CALL);
					Comma(dp->XT);
					MakeTailJmpSafe();
				}
			}
			else
			{
				ExecuteXT(dp->XT);
				//wprintf(_T("%s: cannot execute '%s'!\n"), (LPCTSTR)source, (LPCTSTR)word);
			}
			continue;
		}

		CELL num = 0;
		if (MakeNumber(word, num))
		{
			if (STATE == 0)
			{
				push(num);
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
				push(num);
				continue;
			}
			continue;
		}

		wprintf(_T("%s: '%s'??\n"), (LPCTSTR)source, (LPCTSTR)word);
	}
}

void CCFCompiler::Dis(FILE *fp)
{
	CELL PC = 0;
	while (PC < HERE)
	{
		PC = Dis1(PC, fp);
	}

	fprintf(fp, "\n%0*x ; The dictionary starts here ...\n", CELL_SZ*2, LAST);

	PC = LAST;
	while (PC >= LAST)
	{
		PC = DisDict(PC, fp);
	}
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
	fprintf(fp, "%s\n", (char *)out);
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

	else if (op == BRANCH)
	{
		CELL arg = GetAt(PC);
		DisRange(line, PC, CELL_SZ);
		desc.Format(_T("BRANCH %0*x"), CELL_WD, arg);
		PC += CELL_SZ;
	}

	else if (op == BRANCHZ)
	{
		CELL arg = GetAt(PC);
		DisRange(line, PC, CELL_SZ);
		desc.Format(_T("BRANCHZ %0*x"), CELL_WD, arg);
		PC += CELL_SZ;
	}

	else if (op == BRANCHNZ)
	{
		CELL arg = GetAt(PC);
		DisRange(line, PC, CELL_SZ);
		desc.Format(_T("BRANCHNZ %0*x"), CELL_WD, arg);
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
	if (dp->next == 0)
	{
		line.Format(_T("\n%0*x"), CELL_WD, PC, dp->next);
		DisRange(line, PC, CELL_SZ);
		desc.Format(_T("End"));
		DisOut(fp, line, desc, 33);
		return 0;
	}

	line.Format(_T("\n%0*x"), CELL_WD, PC);
	DisRange(line, PC, CELL_SZ);
	desc.Format(_T("Next=%0*x, %S"), CELL_SZ, dp->next, dp->name);
	DisOut(fp, line, desc, 33);

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

	return dp->next;
}

