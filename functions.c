#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>
#include "globals.h"

int cnt;
void secondPass()
{
	cnt = 0;
	int byteCnt = 0;
	while(cnt < tokensCnt)
	{	
		token* t = tokens[cnt];
		cnt++;

		printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
		//direktive, simboli			
			unsigned int oc = 0;
			int cond;
			if(t->tokenType == SECTION)
			{
				int i;
				for(i = 0; i < sectionTableSize; ++i)
				{
					if(strcmp(t->token, &stringTable[sectionTable[i].st_name]) == 0)
						currSection = i;
				}
				currSection = t->token;
				byteCnt = 0;
			}
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
				byteCnt += 4;
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
				byteCnt += 4;
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
				byteCnt += 4;
			}
			if(prevToken != 0)
			{
				if(strcmp(prevToken->token, ".char") == 0)
					byteCnt += 1;
				if(strcmp(prevToken->token, ".word") == 0)
					byteCnt += 2;
				if(strcmp(prevToken->token, ".long") == 0)
					byteCnt += 4;
				if(strcmp(prevToken->token, ".skip") == 0)
					byteCnt += atoi(tok->token);
				if(strcmp(prevToken->token, ".align") == 0)
				{
					int imm = atoi(tok->token);
					int pad = imm - byteCnt % imm;
					byteCnt += pad;
				}
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

long parseExpression(char* ex)
{
	int cnt = 0;
	int exLen = strlen(ex);
	char* sym = 0;
	int symSize = 0;
	long op1 = 0;
	long op2;
	int numOp = 0;
	char op;

	while(cnt < exLen)
	{
			if(ex[cnt] == '+' || ex[cnt] == '-')
			{
				if(ex[cnt] != ' ')
				op = ex[cnt];
			}
			else if( (ex[cnt] == ' ' || cnt == (exLen - 1)) && sym != 0)
			{
				int i;
				for(i = 0; i < symbolTableSize; i++)
				{
					if(strcmp(sym, &stringTable[symbolTable[i].st_name]) == 0)
					{
						if(op == '+')
							op1 += symbolTable[i].st_value;
						else if(op == '-')
							op1 -= symbolTable[i].st_value;
					}
				}
				free(sym);
				symSize = 0;
			}
			else
			{
				if(symSize == 0)
					sym = (char*) malloc(sizeof(char));
				else
					sym = (char*) realloc(sym, sizeof(char) * (symSize + 1));
				sym[symSize] = ex[cnt];
				symSize++;
			}
			cnt++;

	}
}

long processSymbol(char* sym, char r, int offset)
{
	int i;
	int currSectionIndex = 0;
	int symbolIndex = 0;

	for(i = 1; i < symbolTableSize; i++)
	{
		if(strcmp(sym, &stringTable[symbolTable[i].sh_name]) == 0)
		{
			symbolIndex = i;
			currSectionIndex = symbolTable[symbolIndex].st_shndx
		}
	}

	if(r == 'R' && currSection == symbolTable[symbolIndex].st_shndx)
		return symbolTable[symbolIndex].st_value;

	Elf32_Rel rel;
	rel.r_offset = byteCnt + offset;
	rel.r_info = 0;

	if(r == 'R')
		r_info |= 0x02000000;
	else
		r_info |= 0x01000000;
	r_info != symbolIndex;

	if(strcmp((tokens[cnt - 1]->token, ".char") == 0 || strcmp(tokens[cnt - 1]->token, ".word") == 0 || strcmp(tokens[cnt - 1]->token, ".long") == 0)
	{
		if( (symbolTable[symbolIndex].st_info && 0x01) != 0) //global
		{
			if(dataCount == 0)
				dataSection = (unsigned int*) malloc(sizeof(unsigned int));
			else
				dataSection = (unsigned int*) realloc(dataSection, sizeof(unsigned int) * (dataCount + 1));

			dataCount[dataCount] = 0x00;
			dataCount++;

			return dataSection[dataCount];
		}	
		else//local
		{
			if(dataCount == 0)
				dataSection = (unsigned int*) malloc(sizeof(unsigned int));
			else
				dataSection = (unsigned int*) realloc(dataSection, sizeof(unsigned int) * (dataCount + 1));

			dataSection[dataCount] = symbolTable[symbolIndex].st_value;
			dataCount++;

			return dataSection[dataCount];

		}
	}
	else //nailazak na simbol u insrukciji
	{
		int global = ((symbolTable[symbolIndex].st_info && 0x01) != 0 ) ? 1 : 0;
		if(r == 'R')
		{
			if(global == 1)
			{
				return 0xfcffffff;
			}
		}
		else
		{
			if(global == 1)
			{

			}
			else
			{

			}
		}

	}

}