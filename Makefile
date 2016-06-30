VPATH=/usr/include

prog: main.o functions.o
	gcc -o prog main.o functions.o

main.o: main.c globals.h
	gcc -c -g main.c

functions.o: functions.c globals.h
	gcc -c -g functions.c

clean:
	rm prog *.o