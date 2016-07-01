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

	fseek(file, 0, SEEK_SET);

	secondPass();

	int i;
	for(i = 0; i < instCnt; i++)
		printf("Instr %d\n", textSection[i]);
	printSymbolTable();
	printSectionTable();
}