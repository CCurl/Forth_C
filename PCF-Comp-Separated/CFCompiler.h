#pragma once

#include "..\CFComp-Shared.h"

typedef struct {
	CELL next, XT;
	BYTE flags;
	BYTE len;
	char name[30];
} DICT_T;

class CCFCompiler
{
public:
	CCFCompiler();
	~CCFCompiler();

	void Compile(LPCTSTR m_source, LPCTSTR m_output);
	void GetWord(CString& line, CString& word);
	void DefineWord(CString& word, BYTE flags);
	void Parse(CString& line);
	bool ExecuteOpcode(BYTE opcode);
	BYTE FindAsm(CString& word);
	BYTE FindForthPrim(CString& word);
	DICT_T *FindWord(CString& word);
	OPCODE_T *FindOpcode(BYTE opcode);
	BOOL MakeNumber(CString& word, CELL& the_num);
	void SetAt(CELL loc, CELL num);
	CELL GetAt(CELL loc);
	void Comma(CELL num);
	void CComma(BYTE num);
	void Push(CELL val) { stack[++SP] = val; }
	CELL Pop() { return stack[SP--]; }
	void Dis(FILE *fp);
	CELL Dis1(CELL PC, FILE *fp);
	void DisRange(CString& line, CELL loc, CELL num);
	CELL DisDict(CELL PC, FILE *fp);
	void GetWordName(CELL loc, CString& name);
	bool IsTailJmpSafe();
	void MakeTailJmpSafe();
	void MakeTailJmpUnSafe();

	BYTE the_memory[MEM_SZ];
	CELL HERE, LAST, STATE;
	CELL stack[256], SP;
	CELL line_no;
};
