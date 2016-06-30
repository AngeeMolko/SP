#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>
#include "globals.h"

FILE* file;
char* stringTable;
int strtSize = 0;
int sectionCnt = 0;
token* prevToken = 0;
char* currSection;
Elf32_Sym* symbolTable = 0;
int symbolTableSize = 0;
Elf32_Shdr* sectionTable = 0;
int sectionTableSize = 0;
int sectionIndex;
int typeCnt = 12;
int sectionIndex = 0;
type next = ANY;

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
	if(t == OTHER)
		return "OTHER";

}

int makeSectionTableEntry(token* t, int stringIndex)
{
	//expand section table by 1
	printf("Alociranje sekcije!\n");
	sectionTable = (Elf32_Shdr*) realloc(sectionTable, sizeof(Elf32_Shdr)*(sectionTableSize + 1));

	Elf32_Shdr sec;
	char* name = t->token;

	sec.sh_name = stringIndex; //the rest will be done in makeStringTableEntry

	type myType = t->tokenType;

	if(myType == SECTION)
	{
		sectionIndex = sectionTableSize; // start of a new section

		if(strcmp(name, ".data") == 0 || strcmp(name, ".text") == 0)
			sec.sh_type = SHT_PROGBITS; // there is an existing type for .data and .txt
		else if(strcmp(name, ".bss") == 0)
			sec.sh_type = SHT_NOBITS; // there is an existing type for .bss
		else
		{
			sec.sh_type = typeCnt; // make new type
			typeCnt++; 
		}
	}
	else // myType == SUBSECTION
		sec.sh_type = sectionTable[sectionIndex].sh_type; // subsections are the same type as the section they belong to

	//enter new entry into the section table
	sectionTable[sectionTableSize] = sec;
	sectionTableSize++;

	return sectionTableSize - 1;

}

int makeSymbolTableEntry(token* t)
{
	Elf32_Sym myEntry;

	int index = makeStringTableEntry(t);
	myEntry.st_name = index;
	myEntry.st_value = sectionCnt;
	myEntry.st_other = 0x00; // ??
	myEntry.st_size = 0x00; // ??

	type myType = t->tokenType;

	if(myType == LABEL)
		myEntry.st_info = 0x00; /// ? nznm
	if(myType == SECTION || myType == SUBSECTION)
	{
		next = ANY;
		myEntry.st_info = 0x03;
	}

	printf("Alociranje simbola!\n");
	symbolTable = (Elf32_Sym*) realloc(symbolTable, sizeof(Elf32_Sym)*(symbolTableSize + 1));

	symbolTable[symbolTableSize] = myEntry;
	symbolTableSize++;

	return symbolTableSize - 1;
}

int makeStringTableEntry(token* t)
{
	//+1 because of the terminal character
	int tokenSize = strlen(t->token) + 1;
	int ret = strtSize;
	int i;

	//expand string table by tokenSize
	printf("Alociranje stringa!\n");
	stringTable = (char *) realloc(stringTable, sizeof(char) * (strtSize + tokenSize));

	//copy token name into the table
	for(i = 0; i < (tokenSize - 1); i++)
	{
		stringTable[strtSize] = t->token[i];
		strtSize++;
	}

	stringTable[strtSize] = '\0';
	strtSize++;

	return ret;
}

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

void printSymbolTable()
{
	int i, j;
	int index;
	for(i = 0; i < symbolTableSize; i++)
	{
		index = symbolTable[i].st_name;

		printf("Symbol name: %s\n", &stringTable[index]);
		printf("Symbol value: %d\n", symbolTable[i].st_value);
		printf("Symbol size: %d\n", symbolTable[i].st_size);
		printf("Symbol info: %d\n", symbolTable[i].st_info);
		printf("Symbol other: %d\n", symbolTable[i].st_other);
		printf("Symbol shndx: %d\n", symbolTable[i].st_shndx);
	}
}

void printSectionTable()
{
	int i, j;
	int index;
	for(i = 0; i < sectionTableSize; i++)
	{
		index = sectionTable[i].sh_name;

		printf("Symbol name: %s\n", &stringTable[index]);
		printf("Symbol value: %d\n", sectionTable[i].sh_type);
	}
}
