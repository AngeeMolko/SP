#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include "globals.h"

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

		tok = (char*) realloc(tok, sizeof(char) * (cnt + 1));
		tok[cnt] = c;
		cnt++;
		c = getc(file);

		charCount++;
	}
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

	return tok;
}


int main()
{
	file = fopen("ulaz.txt", "r");
	if(file != 0)
		printf("Fajl otvoren!\n");

	strtSize = 0;

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
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
			else if(strcmp(op, "mov") == 0 || strcmp(op, "shr") == 0 || strcmp(op, "shl") == 0 || strcmp(op, "ldr") == 0 || strcmp(op, "str") == 0)
			{
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
			else
			{
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
				t = createEntry(OPERAND);
				printf("Ja sam %s: %s \n", typeToString(t->tokenType), t->token);
			}
		}

		if(t->end == 1)
			break;
	}
	fclose(file);
	printf("Fajl zatvoren!\n");

	printSymbolTable();
	printSectionTable();
}