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
unsigned int* textSection = 0;
int instCnt = 0;

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
		token* t = createEntry(ANY);

		printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);		
		if(t->tokenType == MNEMONIC)
		{
			char* op = t->token;
			t = createEntry(CONDITION);
			printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			if(strcmp(op, "int") == 0)
			{
				tokens[tokensCnt - 2]->tokenType = MNEMONIC1;
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
			else if(strcmp(op, "shr") == 0 || strcmp(op, "shl") == 0 || strcmp(op, "shl_") == 0 || strcmp(op, "shr_") == 0 ||
					strcmp(op, "ldrib") == 0 || strcmp(op, "ldria") == 0 || strcmp(op, "ldrdb") == 0 || strcmp(op, "ldrda") == 0 || 
					strcmp(op, "strib") == 0 || strcmp(op, "stria") == 0 || strcmp(op, "strdb") == 0 || strcmp(op, "strda") == 0)
			{
				tokens[tokensCnt - 2]->tokenType = MNEMONIC3;
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
			else
			{
				tokens[tokensCnt - 2]->tokenType = MNEMONIC2;
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
		}
		if(t->tokenType == SECTION)
		{
			t = createEntry(EXPRESSION);
			printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
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

void secondPass()
{
	int cnt = 0;
	while(cnt < tokensCnt)
	{	
		token* t = tokens[cnt];
		cnt++;

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
			
			unsigned int oc = 0;
			int cond;
			if(t->tokenType == MNEMONIC1)
			{
				unsigned int update = t->flags;
				unsigned int opCode = getOpCode(t->token);
				t = tokens[cnt];
				cnt++;
				if(t->tokenType == CONDITION)
				{
					cond = condToCode(isCondition(t->token));
					t = tokens[cnt];
					cnt++;
				}
				else
					cond = condToCode(AL);
				oc |= cond;
				oc <<= 1;
				oc |= update;
				oc <<= 4;
				oc |= opCode;
				oc <<= 4;
				unsigned int op1 = atoi(t->token);
				oc <<= 20;
				//provera dve greske, da nije immed i da je vece od 16
				oc |= op1;

				//ne mora text, moze bilo koja sekcija koja sadrzi tekst, za pocetak nek bude samo txt
				if(instCnt == 0)
					textSection = (int*) malloc(sizeof(int));
				else
					textSection = (int*) realloc(textSection, sizeof(int) * (instCnt + 1));
					instCnt++;

					textSection[instCnt - 1] = oc;
			}
			if(t->tokenType == MNEMONIC2)
			{
				unsigned int update = t->flags;
				unsigned int opCode = getOpCode(t->token);
				char* op = t->token;
				t = tokens[cnt];
				cnt++;
				if(t->tokenType == CONDITION)
				{
					cond = condToCode(isCondition(t->token));
					t = tokens[cnt];
					cnt++;
				}
				else
					cond = condToCode(AL);

				oc |= cond;
				oc <<= 1;
				oc |= update;
				oc <<= 4;
				oc |= opCode;
				if(opCode == 13 || opCode == 15)
					oc <<= 4;
				else
					oc <<= 5;
				//provera da li je registar - mora da bude registar?
				unsigned int op1 = isRegister(t->token);
				oc |= op1;
				unsigned int op2;
				t = tokens[cnt];
				cnt++;

				if(opCode == 15)
				{
					if(strcmp(op, "ldc") == 0);
					{
						unsigned int oc2 = oc;
						oc2 <<= 1;
						oc2 |= 0x00000000;
						oc2 <<= 19;
						op2 = atoi(t->token);
						oc2 |= op2;
						if(instCnt == 0)
							textSection = (int*) malloc(sizeof(int));
						else
						textSection = (int*) realloc(textSection, sizeof(int) * (instCnt + 1));

						instCnt++;
						textSection[instCnt - 1] = oc2;
						oc <<= 1;
						oc |= 0x00000001;
					}
					//ldch i ldcl - treba i za ldc!
					if(strcmp(op, "ldch") == 0)
					{
						oc <<= 1;
						oc |= 0x00000001;
					}
					else if (strcmp(op, "ldcl") == 0)
					{
						oc << 1;
						oc |= 0x00000000;
					}
					oc <<= 19;
					if(t->tokenType == IMMEDIATE)
						op2 = atoi(t->token); //sad treba da ide else, pa provera za labelu, table arelokacija ...
					oc |= op2;
				}
				else if(t->tokenType == IMMEDIATE)
				{
					if(opCode == 12) //call
						oc <<= 19;
					else //add, sub, mul, div, cmp - immed
					{
						oc <<= 1;
						oc |= 0x00000001;
						oc <<= 18;
					}
					op2 = atoi(t->token);
					oc |= op2;
				}
				else
				{
					if(opCode <=4)
					{
						oc <<= 1;
						oc || 0x00000000;
					}
					//provera
					op2 = isRegister(t->token);
					if(opCode == 13)
						oc <<= 4;
					else
						oc <<= 5;
					oc |= op2;
					if(opCode <= 4)
						oc <<= 13;
					else if(opCode == 13)
					{
						oc <<= 1;
						if(strcmp(op, "in") == 0)
							oc |= 0x00000001;
						else
							oc |= 0x00000000;
						oc <<= 15;
					}
					else
						oc <<= 14;
				}
				//kod ldcl, ldch c moze da bude i labela, ne samo immed, kako to uraditii
				if(instCnt == 0)
						textSection = (int*) malloc(sizeof(int));
				else
					textSection = (int*) realloc(textSection, sizeof(int) * (instCnt + 1));
				instCnt++;

				textSection[instCnt - 1] = oc;
			}
			if(t->tokenType == MNEMONIC3)
			{
				unsigned int update = t->flags;
				unsigned int opCode = getOpCode(t->token);
				char* op = t->token;
				t = tokens[cnt];
				cnt++;
				if(t->tokenType == CONDITION)
				{
					cond = condToCode(isCondition(t->token));
					t = tokens[cnt];
					cnt++;
				}
				else
					cond = condToCode(AL);

				oc |= cond;
				oc <<= 1;
				oc |= update;
				oc <<= 4;
				oc |= opCode;
				oc <<= 5;

				//provere za sve
				int op1 = isRegister(t->token);
				t = tokens[cnt];
				cnt++;
				int op2 = isRegister(t->token);
				t = tokens[cnt];
				cnt++;
				int op3 = atoi(t->token);
				oc |= op1;
				oc <<= 5;
				oc |= op2;
				unsigned int f;
				if(strcmp(op, "ldrib") == 0 || strcmp(op, "strib") == 0)
				{
					if(op1 == PC || op2 == PC)
						f = 0;
					else
						f = 4;
					oc <<= 3;
					oc |= f;
					oc <<= 1;
					if(strcmp(op, "ldrib") == 0)
						oc |= 0x00000001;
					else
						oc |= 0x00000000;
					oc <<= 10;
					oc |= op3;

				}
				else if(strcmp(op, "ldria") == 0 || strcmp(op, "stria") == 0)
				{
					if(op1 == PC || op2 == PC)
						f = 0;
					else
						f = 2;
					oc <<= 3;
					oc |= f;
					oc <<= 1;
					if(strcmp(op, "ldria") == 0)
						oc |= 0x00000001;
					else
						oc |= 0x00000000;
					oc <<= 10;
					oc |= op3;
				}
				else if(strcmp(op, "ldrda") == 0 || strcmp(op, "strda") == 0)
				{
					if(op1 == PC || op2 == PC)
						f = 0;
					else
						f = 3;
					oc <<= 3;
					oc |= f;
					oc <<= 1;
					if(strcmp(op, "ldrda") == 0)
						oc |= 0x00000001;
					else
						oc |= 0x00000000;
					oc <<= 10;
					oc |= op3;
				}
				else if(strcmp(op, "ldrdb") == 0 || strcmp(op, "strdb") == 0)
				{
					if(op1 == PC || op2 == PC)
						f = 0;
					else
						f = 5;
					oc <<= 3;
					oc |= f;
					oc <<= 1;
					if(strcmp(op, "ldrib") == 0)
						oc |= 0x00000001;
					else
						oc |= 0x00000000;
					oc <<= 10;
					oc |= op3;
				}
				else if(strcmp(op, "mov") == 0 || strcmp(op, "mov_") == 0)
				{
					oc << 14;
					oc |= 0x00000000;
				}
				else
				{
					oc <<= 5;
					oc |= op3;
					oc <<= 1;
					if(strcmp(op, "shr") == 0 || strcmp(op, "shr_") == 0)
						oc |= 0x00000000;
					else
						oc |= 0x00000001;
					oc <<= 8;
				}

				if(instCnt == 0)
					textSection = (int*) malloc(sizeof(int));
				else
					textSection = (int*) realloc(textSection, sizeof(int) * (instCnt + 1));

				instCnt++;
				textSection[instCnt - 1] = oc;
			}
	//labele se ignorisu
	//others - char, word, long - posle, align i skip ??
	//mnemonici i operandi


		//izmenitiiii

	if(t->tokenType != SYMBOL && t->tokenType != OPERAND && t->tokenType != CONDITION)
		prevToken = t;

	if(t->end == 1)
		break;
	}
}
