#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100

typedef enum {ANY, MNEMONIC, LABEL, OPERAND, IMMEDIATE, DIRECTIVE, OTHER, SECTION, SUBSECTION, ENDOFFILE, SYMBOL, CONDITION, END, ERROR} type;
typedef enum {NO, AL, EQ, NE, GT, GE, LT, LE} condition;
typedef struct Token {
	char* token;
	type tokenType;
	int end;
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

char* typeToString(type t);

int makeSectionTableEntry(token* t, int index);
int makeSymbolTableEntry(token* t);
int makeStringTableEntry(token* t);

void printSymbolTable();
void printSectionTable();