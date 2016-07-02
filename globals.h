#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100

typedef enum {ANY, MNEMONIC, MNEMONIC1, MNEMONIC2, MNEMONIC3, LABEL, EXPRESSION, OPERAND, IMMEDIATE, DIRECTIVE, OTHER, SECTION, SUBSECTION, ENDOFFILE, SYMBOL, CONDITION, END, ERROR} type;
typedef enum {NO, AL, EQ, NE, GT, GE, LT, LE} condition;
typedef enum {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, PC, LR, SP, PSW, NONE} reg;
typedef enum {NOTMNEMONIC, UPDATE, NOUPDATE} mnemonic;
typedef struct Token {
	char* token;
	type tokenType;
	int end;
	int flags;
} token;


extern FILE* file;
extern char* stringTable;
extern int strtSize;
extern int sectionCnt;
extern token* prevToken;
extern char* currSection;
extern Elf32_Sym* symbolTable;
extern int symbolTableSize;
extern Elf32_Shdr* sectionTable;
extern int sectionTableSize;
extern int sectionIndex;
extern int typeCnt;
extern int sectionIndex;
extern type next;
extern token** tokens;
extern int tokensCnt;
extern unsigned int* textSection;
extern int instCnt;

void printSymbolTable();
void printSectionTable();

void firstPass();
void secondPass();

//decodeFunctions.c
unsigned int getOpCode(char* op);
int condToCode(condition c);
reg isRegister(char* op);
mnemonic isMnemonic(char* tok);
condition isCondition(char* cond);
char* typeToString(type t);