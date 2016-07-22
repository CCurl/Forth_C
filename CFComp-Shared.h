#pragma once

#include "Shared.h"

typedef struct {
	CString asm_instr;
	BYTE opcode;
	CString forth_prim;
} OPCODE_T;
