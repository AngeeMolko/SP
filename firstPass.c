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
int currSection = 0;
symbol* symbolTable = 0;
int symbolTableSize = 0;
Elf32_Shdr* sectionTable = 0;
int sectionTableSize = 0;
int sectionIndex;
int typeCnt = 12;
int sectionIndex = 0;
type next = ANY;
token** tokens = 0;
int tokensCnt = 0;
int opCnt = 0;
unsigned int* textSection = 0;
int instCnt = 0;
unsigned int* dataSection = 0;
int dataCount = 0;
unsigned int** sectionContent = 0;
int* sectionContentCnt = 0;
rel** relocationTable = 0;
int* relocationTableSize = 0;

int makeSectionTableEntry(token* t, int stringIndex);
int makeSymbolTableEntry(token* t);
int makeStringTableEntry(token* t);
token* createEntry(type t);
token* makeToken(type ty);

int makeSectionTableEntry(token* t, int stringIndex)
{
	//expand section table by 1
	printf("Alociranje sekcije!\n");
	if(sectionTableSize == 0)
	{
		sectionTable = (Elf32_Shdr*) malloc(sizeof(Elf32_Shdr));
		sectionTableSize++;
	}
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
	symbol myEntry;

	int index = makeStringTableEntry(t);
	printf("Napravio string! %d \n", sectionTableSize);
	myEntry.st_name = index;
	if(prevToken != 0)
	{
		if((strcmp(prevToken->token, ".public") == 0 || strcmp(prevToken->token, ".extern") == 0) && t->tokenType == SYMBOL)
		{
			myEntry.is_global = 1;
			myEntry.st_value = 0;
			myEntry.sectionIndex = 0;
		}
		else
		{
			myEntry.is_global = 0;
			myEntry.st_value = sectionCnt;
			myEntry.sectionIndex = currSection;
		}
	}
	if((t->tokenType == SECTION || t->tokenType == SUBSECTION) && sectionTableSize != 0)
	{
		int i;
		for(int i = 0; i < symbolTableSize; ++i)
		{
			if(symbolTable[i].sectionIndex == (sectionTableSize - 1))
			{
				symbolTable[i].st_size = sectionTableSize;
				break;
			}
		}
	}
	else
	{
			myEntry.is_global = 0;
			myEntry.st_value = sectionCnt;
			myEntry.sectionIndex = currSection;
	}
	if(prevToken != 0)
	{
		if(strcmp(prevToken->token, ".char") == 0)
			myEntry.st_size = 0x01;
		if(strcmp(prevToken->token, ".word") == 0)
			myEntry.st_size = 0x02;
		if(strcmp(prevToken->token, ".long") == 0)
			myEntry.st_size = 0x04;
		else
			myEntry.st_size = 0x00;
	}
	else
		myEntry.st_size = 0x00;
	if(prevToken != 0)
	{
		if(strcmp(prevToken->token, ".extern") == 0)
			myEntry.is_extern = 1;
	}
	else
		myEntry.is_extern = 0;

	type myType = t->tokenType;

	if(myType == SECTION || myType == SUBSECTION)
	{
		next = ANY;
		myEntry.is_section = 1;
	}
	else
		myEntry.is_section = 0;


	printf("Alociranje simbola!\n");
	symbolTable = (symbol*) realloc(symbolTable, sizeof(symbol)*(symbolTableSize + 1));

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
		printf("Symbol shndx: %d\n", symbolTable[i].sectionIndex);
		printf("Is global: %d\n", symbolTable[i].is_global);
		printf("Is section: %d\n", symbolTable[i].is_section);
	}
}

void printSectionTable()
{
	int i, j;
	int index;
	for(i = 1; i < sectionTableSize; i++)
	{
		index = sectionTable[i].sh_name;

		printf("Section name: %s\n", &stringTable[index]);
		printf("Section value: %d\n", sectionTable[i].sh_type);
	}
}

void firstPass()
{
	token* t = createEntry(ANY);
	while(1)
	{	
		printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);		
		if(t->tokenType == MNEMONIC)
		{
			char* op = t->token;
			if(strcmp(op, "int") == 0)
			{
				t->tokenType = MNEMONIC1;
				prevToken = t;
				t = createEntry(OPERAND);
				if(t->tokenType == CONDITION)
				{
					printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
					t = createEntry(OPERAND);
				}
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
			else if(strcmp(op, "shr") == 0 || strcmp(op, "shl") == 0 || strcmp(op, "shl_") == 0 || strcmp(op, "shr_") == 0 ||
					strcmp(op, "ldrib") == 0 || strcmp(op, "ldria") == 0 || strcmp(op, "ldrdb") == 0 || strcmp(op, "ldrda") == 0 || 
					strcmp(op, "strib") == 0 || strcmp(op, "stria") == 0 || strcmp(op, "strdb") == 0 || strcmp(op, "strda") == 0)
			{
				t->tokenType = MNEMONIC3;
				prevToken = t;
				t = createEntry(OPERAND);
				if(t->tokenType == CONDITION)
				{
					printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
					t = createEntry(OPERAND);
				}
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
			else
			{
				t->tokenType = MNEMONIC2;
				prevToken = t;
				t = createEntry(OPERAND);
				if(t->tokenType == CONDITION)
				{
					printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
					t = createEntry(OPERAND);
				}
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
		}
		else if(t->tokenType != SYMBOL) /////////////////mozda je ovde greska
			prevToken = t;

		if(t->end == 1)
			break;

		t = createEntry(ANY);


	}
	prevToken = 0;
}

token* createEntry(type t)
{
	token* tok = makeToken(t);
	type myType = tok->tokenType;
	if(myType == END)
		return tok;

	if(myType == SECTION || myType == SUBSECTION || myType == LABEL) // new symbol table entry needed
	{
		if(myType == SECTION || myType == SUBSECTION)
			sectionCnt = 0;

		int i;
		int symbolIndex;
		int is_public = 0;
		if(myType == LABEL)
		{
			for(i = 0; i < symbolTableSize; ++i)
			{
				if(strcmp(tok->token, &stringTable[symbolTable[i].st_name]) == 0)
				{	
					symbolIndex = i;
					is_public = 1;
				}
			}
			if(is_public == 1)
			{
				symbolTable[symbolIndex].st_value = sectionCnt;
				symbolTable[symbolIndex].is_global = 1;
				symbolTable[symbolIndex].is_section = 0;
				symbolTable[symbolIndex].sectionIndex = currSection;
				symbolTable[symbolIndex].st_size = 0;
			}
			else
			{
				symbolIndex = makeSymbolTableEntry(tok);
				symbolTable[symbolIndex].sectionIndex = currSection;
			}
		}	
		else if(myType == SECTION || myType == SUBSECTION)
		{
			symbolIndex = makeSymbolTableEntry(tok);
			sectionIndex = makeSectionTableEntry(tok, symbolTable[symbolIndex].st_name);
			symbolTable[symbolTableSize - 1].sectionIndex = sectionIndex;
			currSection = sectionIndex;
			symbolTable[symbolIndex].sectionIndex = sectionIndex;
		}

		//printSymbolTable();
		//printSectionTable();
	}
	else if(prevToken != 0)
	{
		if( (strcmp(prevToken->token, ".extern") == 0) && tok->tokenType == SYMBOL)
		{
			int symbolIndex = makeSymbolTableEntry(tok);
			symbolTable[symbolIndex].sectionIndex = 0; /// ????
			symbolTable[symbolIndex].is_extern = 1;
			symbolTable[symbolIndex].is_global = 1;
		}
		if(strcmp(prevToken->token, ".public") == 0 && tok->tokenType == SYMBOL)
		{
			int i;
			int is_defined = 0;
			for(i = 0; i < symbolTableSize; i++)
			{
				if(strcmp(tok->token, &stringTable[symbolTable[i].st_name]) == 0)
				{
					is_defined = 1;
					break;
				}
			}
			if(is_defined == 1)
				symbolTable[i].is_global = 1;
			else
			{
				int symbolIndex = makeSymbolTableEntry(tok);
				symbolTable[symbolIndex].sectionIndex = 0; /// ????
				symbolTable[symbolIndex].is_extern = 0;
				symbolTable[symbolIndex].is_global = 1;
			}
		}

	}

	if(myType == MNEMONIC)
		sectionCnt += 4;
	if(prevToken != 0)
	{
		if(strcmp(prevToken->token, ".char") == 0)
			sectionCnt += 1;
		if(strcmp(prevToken->token, ".word") == 0)
			sectionCnt += 2;
		if(strcmp(prevToken->token, ".long") == 0)
			sectionCnt += 4;
		if(strcmp(prevToken->token, ".skip") == 0)
			sectionCnt += atoi(tok->token);
		if(strcmp(prevToken->token, ".align") == 0)
		{
			int imm = atoi(tok->token);
			int pad = imm - sectionCnt % imm;
			sectionCnt += pad;
		}
	}

	// if(tok->tokenType != SYMBOL)
	// 	prevToken = tok;

	if(tokensCnt == 0)
		tokens = (token**) malloc(sizeof(token*));
	else
		tokens = (token**) realloc(tokens, sizeof(token*) * (tokensCnt + 1));

	tokens[tokensCnt] = tok;
	tokensCnt++;

	return tok;
}

token* makeToken(type ty)
{
	char* tok = 0;
	char c = getc(file);
	int cnt = 0;

	token* t = (token *) malloc(sizeof(token));

	while(c == ' ' || c == '\n')
		c = getc(file);

	int dotCount = 0;
	if(ty == EXPRESSION)
	{
		while(c != '\n')
		{
			if(tok == 0)
				tok = (char *) malloc(sizeof(char) * 1);
			else
				tok = (char*) realloc(tok, sizeof(char) * (cnt + 1));
			tok[cnt] = c;
			cnt++;
			c = getc(file);
		}
	}
	else 
	{
		while(c != ',' && c != ':' && c != ' ' && c != '\n' && c!= EOF)
		{
			if(c == '.')
				dotCount++;
			if(tok == 0)
				tok = (char *) malloc(sizeof(char) * 1);
			else
				tok = (char*) realloc(tok, sizeof(char) * (cnt + 1));
			tok[cnt] = c;
			cnt++;
			c = getc(file);
		}
	}
	tok = (char*) realloc(tok, sizeof(char) * (cnt + 1));
	tok[cnt] = '\0';

	t->token = tok;

	if (c == EOF)
		t->end = 1;
	if(ty == EXPRESSION)
	{
		t->tokenType = EXPRESSION;
		return t;
	}
	if(strcmp(tok, ".end") == 0)
	{
		t->tokenType = END;
		return t;
	}
	else if(tok[0] == '.')
	{
		if(strcmp(tok, ".public") == 0 || strcmp(tok, ".extern") == 0)
			t->tokenType = DIRECTIVE;
		else if(strcmp(tok, ".char") == 0 || strcmp(tok, ".long") == 0 || strcmp(tok, ".word") == 0 || strcmp(tok, ".align") == 0 || strcmp(tok, ".skip") == 0 )
			t->tokenType = OTHER;
		else if(dotCount == 2)
			t->tokenType = SUBSECTION;
		else
			t->tokenType = SECTION;
		return t;
	}
	else if(tok[0] >= '0' && tok[0] <= '9')
	{
		t->tokenType = IMMEDIATE;
		return t;
	}
	else if(c == ':')
	{
		t->tokenType = LABEL;
		return t;
	}
	else if(isCondition(tok) != NO)
	{
		t->tokenType = CONDITION;
		return t;
	}
	if(ty == OPERAND)
	{	
		t->tokenType = OPERAND;
		return t;
	}
	else if(isMnemonic(tok) == UPDATE)
	{
		t->tokenType = MNEMONIC;
		t->flags = 1;
	}
	else if(isMnemonic(tok) == NOUPDATE)
	{
		t->tokenType = MNEMONIC;
		t->flags = 0;
	}
	else	
		t->tokenType = SYMBOL;

	return t;
}