#pragma once

#include "..\CForth2\Defines.h"

#define MEM_SZ 8192


typedef struct {
	CString asm_instr;
	int opcode;
	CString forth_prim;
} OPCODE_T;


class CCFCompiler
{
public:
	CCFCompiler();
	~CCFCompiler();

	void Compile(LPCTSTR m_source, LPCTSTR m_output);
	void GetWord(CString& line, CString& word);
	void DefineWord(CString& word, int flags);
	void Parse(CString& line);
	int FindAsm(CString& word);
	int FindForthPrim(CString& word);
	int FindWord(CString& word);
	OPCODE_T *FindOpcode(int opcode);
	int MakeNumber(CString& word, int& the_num);
	void SetAt(int loc, int num);
	int GetAt(int loc);
	void Comma(int num);
	void CComma(int num);
	void Push(int val) { stack[++SP] = val; }
	int Pop() { return stack[SP--]; }
	void Dis(FILE *fp);
	int Dis1(int PC, FILE *fp);
	void DisRange(CString& line, int loc, int num);
	int DisDict(int PC, FILE *fp);
	void GetWordName(int loc, CString& name);

	BYTE the_memory[MEM_SZ];
	int HERE, LAST, STATE;
	int stack[256], SP;
	int line_no;
};
