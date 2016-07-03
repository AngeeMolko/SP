#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include "globals.h"

int main()
{
	file = fopen("ulaz.txt", "r");
	if(file != 0)
		printf("Fajl otvoren!\n");

	firstPass();

	fclose(file);
	printf("Fajl zatvoren!\n");

	secondPass();

	int i;
	for(i = 0; i < instCnt; i++)
		printf("Instr %x\n", textSection[i]);
	printSymbolTable();
	printSectionTable();

	if(relocationTable != 0)
		printf("%d", relocationTable[1][0].symbol);
}