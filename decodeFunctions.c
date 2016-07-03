#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>
#include "globals.h"

condition isCondition(char* cond)
{
	if(strcmp(cond, "eq") == 0)
		return EQ;
	if(strcmp(cond, "ne") == 0)
		return NE;
	if(strcmp(cond, "gt") == 0)
		return GT;
	if(strcmp(cond, "ge") == 0)
		return GE;
	if(strcmp(cond, "lt") == 0)
		return LT;
	if(strcmp(cond, "le") == 0)
		return LE;
	if(strcmp(cond, "al") == 0)
		return AL;
	else
		return NO;
}

mnemonic isMnemonic(char* tok)
{
	if(strcmp(tok, "add") == 0 || strcmp(tok, "cmp") == 0 || strcmp(tok, "sub") == 0 || strcmp(tok, "test") == 0 ||
	   strcmp(tok, "mul") == 0 || strcmp(tok, "div") == 0 || strcmp(tok, "int") == 0 || strcmp(tok, "mov") == 0 || 
	   strcmp(tok, "shr") == 0 || strcmp(tok, "shl") == 0 || strcmp(tok, "and") == 0 || strcmp(tok, "or") == 0 ||
	   strcmp(tok, "not") == 0 || strcmp(tok, "in") == 0 || strcmp(tok, "out") == 0 ||
	   strcmp(tok, "ldrib") == 0 || strcmp(tok, "ldria") == 0 || strcmp(tok, "ldrdb") == 0 || strcmp(tok, "ldrda") == 0 ||
	   strcmp(tok, "strib") == 0 || strcmp(tok, "stria") == 0 || strcmp(tok, "strdb") == 0 || strcmp(tok, "strda") == 0 ||
	   strcmp(tok, "call") == 0 || strcmp(tok, "ldcl") == 0 || strcmp(tok, "ldch") == 0 || strcmp(tok, "ldc") == 0)
			return UPDATE;
	if(strcmp(tok, "add_") == 0 || strcmp(tok, "cmp_") == 0 || strcmp(tok, "sub_") == 0 || strcmp(tok, "test_") == 0 ||
	   strcmp(tok, "mul_") == 0 || strcmp(tok, "div_") == 0 || strcmp(tok, "mov_") == 0 || strcmp(tok, "shr_") == 0 || 
	   strcmp(tok, "shl_") == 0 || strcmp(tok, "and_") == 0 || strcmp(tok, "or_") == 0 || strcmp(tok, "not_") == 0 )
			return NOUPDATE;
	return NOTMNEMONIC;
}

reg isRegister(char* op)
{
	if(strcmp(op, "r0") == 0)
		return R0;
	if(strcmp(op, "r1") == 0)
		return R1;
	if(strcmp(op, "r2") == 0)
		return R2;
	if(strcmp(op, "r3") == 0)
		return R3;
	if(strcmp(op, "r4") == 0)
		return R4;
	if(strcmp(op, "r5") == 0)
		return R5;
	if(strcmp(op, "r6") == 0)
		return R6;
	if(strcmp(op, "r7") == 0)
		return R7;
	if(strcmp(op, "r8") == 0)
		return R8;
	if(strcmp(op, "r9") == 0)
		return R9;
	if(strcmp(op, "r10") == 0)
		return R10;
	if(strcmp(op, "r11") == 0)
		return R11;
	if(strcmp(op, "r12") == 0)
		return R12;
	if(strcmp(op, "r13") == 0)
		return R13;
	if(strcmp(op, "r14") == 0)
		return R14;
	if(strcmp(op, "r15") == 0)
		return R15;
	if(strcmp(op, "pc") == 0)
		return PC;
	if(strcmp(op, "lr") == 0)
		return LR;
	if(strcmp(op, "sp") == 0)
		return SP;
	if(strcmp(op, "psw") == 0)
		return PSW;
	else
		return NONE;
}

int condToCode(condition c)
{
	if(c == NO)
		return 6;
	if(c == EQ)
		return 0;
	if(c == NE)
		return 1;
	if(c == GT)
		return 2;
	if(c == GE)
		return 3;
	if(c == LT)
		return 4;
	if(c == LE)
		return 5;
	if(c == AL)
		return 7;	
}

unsigned int getOpCode(char* op)
{
	if(strcmp(op, "int") == 0)
		return 0;
	if(strcmp(op, "add") == 0 || strcmp(op, "add_") == 0)
		return 1;
	if(strcmp(op, "sub") == 0 || strcmp(op, "sub_") == 0)
		return 2;
	if(strcmp(op, "mul") == 0 || strcmp(op, "mul_") == 0)
		return 3;
	if(strcmp(op, "div") == 0 || strcmp(op, "div_") == 0)
		return 4;
	if(strcmp(op, "cmp") == 0 || strcmp(op, "cmp_") == 0)
		return 5;
	if(strcmp(op, "and") == 0 || strcmp(op, "and_") == 0)
		return 6;
	if(strcmp(op, "or") == 0 || strcmp(op, "or_") == 0)
		return 7;
	if(strcmp(op, "nor") == 0 || strcmp(op, "nor_") == 0)
		return 8;
	if(strcmp(op, "test") == 0 || strcmp(op, "test_") == 0)
		return 9;
	if(strcmp(op, "ldrib") == 0 || strcmp(op, "ldria") == 0 || strcmp(op, "ldrdb") == 0 || strcmp(op, "ldrda") == 0)
		return 10;
	if(strcmp(op, "strib") == 0 || strcmp(op, "stria") == 0 || strcmp(op, "strdb") == 0 || strcmp(op, "strda") == 0)
		return 10;
	if(strcmp(op, "call") == 0)
		return 12;
	if(strcmp(op, "in") == 0)
		return 13;
	if(strcmp(op, "out") == 0)
		return 13;
	if(strcmp(op, "mov") == 0 || strcmp(op, "mov_") == 0)
		return 14;
	if(strcmp(op, "shl") == 0 || strcmp(op, "shl_") == 0)
		return 14;
	if(strcmp(op, "shr") == 0 || strcmp(op, "shr_") == 0)
		return 14;
	if(strcmp(op, "ldch") == 0)
		return 15;
	if(strcmp(op, "ldcl") == 0)
		return 15;
	if(strcmp(op, "ldc") == 0)
		return 15;
	else
		return -1;
}

char* typeToString(type t)
{
	if(t == OPERAND)
		return "OPERAND";
	if(t == ANY)
		return "ANY";
	if(t == ERROR)
		return "ERROR";
	if(t == LABEL)
		return "LABEL";
	if(t == MNEMONIC)
		return "MNEMONIC";
	if(t == DIRECTIVE)
		return "DIRECTIVE";
	if(t == SECTION)
		return "SECTION";
	if(t == 	SUBSECTION)
		return "SUBSECTION";
	if(t == 	IMMEDIATE)
		return "IMMEDIATE";
	if(t == 	SYMBOL)
		return "SYMBOL";
	if(t == 	END)
		return "END";
	if(t == 	CONDITION)
		return "COND";
	if(t ==     MNEMONIC1)
		return "MNEMONIC1";
	if(t ==     MNEMONIC2)
		return "MNEMONIC2";
	if(t ==     MNEMONIC3)
		return "MNEMONIC3";
	if(t ==     EXPRESSION)
		return "EXPRESSION";
	if(t == OTHER)
		return "OTHER";

}

unsigned int toLittleEndian(unsigned int data)
{
	return data;
}