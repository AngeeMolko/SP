VPATH=/usr/include

prog: main.o functions.o decodeFunctions.o firstPass.o
	gcc -o prog main.o functions.o decodeFunctions.o firstPass.o

main.o: main.c globals.h
	gcc -c -g main.c

functions.o: functions.c globals.h decodeFunctions.c firstPass.c
	gcc -c -g functions.c

decodeFunctions.o: decodeFunctions.c globals.h
	gcc -c -g decodeFunctions.c

firstPass.o: firstPass.c globals.h decodeFunctions.c
	gcc -c -g firstPass.c

clean:
	rm prog *.o