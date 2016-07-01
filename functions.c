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
token** tokens = 0;
int tokensCnt = 0;
int opCnt = 0;

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
	for(i = 1; i < sectionTableSize; i++)
	{
		index = sectionTable[i].sh_name;

		printf("Section name: %s\n", &stringTable[index]);
		printf("Section value: %d\n", sectionTable[i].sh_type);
	}
}

void firstPass()
{
	while(1)
	{	
		token* t = createEntry(next);

		printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);		
		if(t->tokenType == MNEMONIC)
		{
			char* op = t->token;
			t = createEntry(CONDITION);
			printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			if(strcmp(op, "int") == 0)
			{
				tokens[tokensCnt - 1]->tokenType = MNEMONIC1;
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
			else if(strcmp(op, "mov") == 0 || strcmp(op, "shr") == 0 || strcmp(op, "shl") == 0 || strcmp(op, "ldr") == 0 || strcmp(op, "str") == 0)
			{
				tokens[tokensCnt - 1]->tokenType = MNEMONIC3;
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
			else
			{
				tokens[tokensCnt - 1]->tokenType = MNEMONIC2;
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
		}

		if(t->end == 1)
			break;
	}
	prevToken = 0;
}

token* createEntry(type t)
{
	token* tok = makeToken(t);
	type myType = tok->tokenType;

	if(myType == SECTION || myType == SUBSECTION || myType == LABEL) // new symbol table entry needed
	{
		if(myType == SECTION || myType == SUBSECTION)
		{
			currSection = tok->token;
			sectionCnt = 0;
		}
		int symbolIndex = makeSymbolTableEntry(tok);
		if(myType == SECTION || myType == SUBSECTION)
			sectionIndex = makeSectionTableEntry(tok, symbolTable[symbolIndex].st_name);

		symbolTable[symbolIndex].st_shndx = sectionIndex;

		printSymbolTable();
		printSectionTable();
	}
	else if(prevToken != 0)
	{
		if( strcmp(prevToken->token, ".extern") == 0 && tok->tokenType == SYMBOL)
		{
			int symbolIndex = makeSymbolTableEntry(tok);
			symbolTable[symbolIndex].st_shndx = 0;
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

	if(tok->tokenType != SYMBOL)
		prevToken = tok;

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

	int charCount = 0;;
	int dotCount = 0;
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

		charCount++;
	}
	tok = (char*) realloc(tok, sizeof(char) * (cnt + 1));
	tok[cnt] = '\0';

	t->token = tok;

	if (c == EOF)
		t->end = 1;

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
	else if(prevToken != 0)
	{
		if(prevToken->tokenType == MNEMONIC)
		{
			t->tokenType = CONDITION;
			return t;
		}
	}
	if(ty == OPERAND)
	{	
		t->tokenType = OPERAND;
		return t;
	}
	else if(strcmp(tok, "add") == 0 || strcmp(tok, "cmp") == 0 || strcmp(tok, "sub") == 0 || strcmp(tok, "test") == 0 ||
		    strcmp(tok, "mul") == 0 || strcmp(tok, "div") == 0 || strcmp(tok, "int") == 0 || strcmp(tok, "mov") == 0 || 
		    strcmp(tok, "shr") == 0 || strcmp(tok, "shl") == 0 || strcmp(tok, "and") == 0 || strcmp(tok, "or") == 0 ||
		    strcmp(tok, "not") == 0 || strcmp(tok, "ldr") == 0 || strcmp(tok, "str") == 0 || strcmp(tok, "in") == 0 || 
		    strcmp(tok, "out") == 0 || strcmp(tok, "call") == 0 || strcmp(tok, "ldcl") == 0 || strcmp(tok, "ldch") == 0)
		t->tokenType = MNEMONIC;
	else	
		t->tokenType = SYMBOL;

	return t;
}

register isRegister(char* op)
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

int condToCode(cond c)
{
	if(cond == NO)
		return 6;
	if(cond == EQ)
		return 0;
	if(cond == NE)
		return 1;
	if(cond == GT)
		return 2;
	if(cond == GE)
		return 3;
	if(cond == LT)
		return 4;
	if(cond == LE)
		return 5;
	if(comd == AL)
		return 7;	
}

void secondPass()
{
	int cnt = 0;
	while(cnt < tokensCnt)
	{	
		token* t = tokens[cnt];

		printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
		//direktive, simboli
		if(prevToken != 0)
		{
			if((strcmp(prevToken->token, ".extern") == 0 || strcmp(prevToken->token, ".public") == 0) && t->tokenType == SYMBOL)
			{
				//dodati kada public nije bio definisan dodavanje u tabelu simbola
				char* name;
				int i;

				for(i = 0; i < symbolTableSize; ++i)
				{
					int index = symbolTable[i].st_name;
					name = &stringTable[index];

					if(strcmp(t->token, name) == 0)
					{
						symbolTable[i].st_info &= 0x01;
						break;
					}
				}
			}
		}	
		if(prevToken->tokenType == MNEMONIC1)
		{
			//ocekuje se immed
			if(opCnt == 1)
			{
				int oc = 0;
				int cond = condToCode(isCondition(tokens[tokensCnt - 2]));

				oc &= cond;
				oc << 1;
				/// ne znam kada ovde treba 1 a kada 0
				oc &= 1;
				oc << 4;
				oc &= 0;
				oc << 4;
				oc &= atoi(tokens[tokensCnt - 1]->token);
				oc << 22;
			}
			else
				opCnt++;
		}
		else if(prevToken->tokenType == MNEMONIC2)
		{
			if(opCnt == 2)
			{
				int oc = 0;
				int cond = condToCode(isCondition(tokens[tokensCnt - 2]));

				oc &= cond;
				oc << 1;
				/// ne znam kada ovde treba 1 a kada 0
				oc &= 1;
				oc << 4;
				int opCode;
				if(strcmp(prevToken->token, "add") == 0)
					opCode = 1;
				if(strcmp(prevToken->token, "sub") == 0)
					opCode = 2;
				if(strcmp(prevToken->token, "mul") == 0)
					opCode = 3;
				if(strcmp(prevToken->token, "div") == 0)
					opCode = 4;
				if(strcmp(prevToken->token, "cmp") == 0)
					opCode = 5;
				if(strcmp(prevToken->token, "and") == 0)
					opCode = 6;
				if(strcmp(prevToken->token, "or") == 0)
					opCode = 7;
				if(strcmp(prevToken->token, "nor") == 0)
					opCode = 8;
				if(strcmp(prevToken->token, "test") == 0)
					opCode = 9;
				if(strcmp(prevToken->token, "call") == 0)
					opCode = 12;
				if(strcmp(prevToken->token, "in") == 0)
					opCode = 13;
				if(strcmp(prevToken->token, "out") == 0)
					opCode = 13;
				if(strcmp(prevToken->token, "ldch") == 0)
					opCode = 15;
				if(strcmp(prevToken->token, "ldcl") == 0)
					opCode = 15;

				oc &= opCode;
				if(opCode == 13 || opCode = 15)
					oc << 4;
				else
					oc << 5;

				int op1 = isRegister(tokens[tokensCnt - 2]->token);
				oc &= op1;
				if(opCode == 15)
				{
					oc << 1;
					if(strcmp(prevToken, "ldch") == 0)
						oc &= 1;
					else
						oc &= 0;
					oc << 3;
				}
				int op2;
				if(opCode <= 4)
				{
					oc << 1;
					oc &= 1;
				}
				else if(opCode == 5)
				{
					oc << 1;
					oc &= 0;
				}
				if(t->tokenType == IMMED)
				{
					if(opCode == 12)
						oc << 19;
					else if(opCode == 15)
						oc << 16;
					else
						oc << 18;
					op2 = atoi(t->token);
					oc&= op2;
				}
				else
				{
					op2 = isRegister(t->token);
					if(opCode == 13)
						oc << 4;
					else
						oc << 5;
					oc &= op2;
					if(opCode <= 4)
						oc << 13;
					if(opCode == 13)
					{
						if(strcmp(prevToken->token, "in") == 0)
							oc &= 1;
						else
							oc &= 0;
						oc << 16;
					}
				}

				//kod ldcl, ldch c moze da bude i labela, ne samo immed, kako to uraditii

				opCnt = 0;
			}
			else
				opCnt++;
		}
		else if(prevToken->tokenType == MNEMONIC3)
		{
			if(opCnt == 3)
			{
				opCnt = 0;
			}
			else
				opCnt++;
		}
	//labele se ignorisu
	//others - char, word, long - posle, align i skip ??
	//mnemonici i operandi


		//izmenitiiii

		if(t->tokenType != SYMBOL && t->tokenType != OPERAND)
			prevToken = t;

		if(t->end == 1)
			break;
	}
}