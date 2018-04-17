CFLAGS= -Wall -Werror

sudoku-serial: sudoku-serial.o list.o
	gcc $(CFLAGS) -o sudoku-serial list.o sudoku-serial.o

sudoku-serial.o: sudoku-serial.c list.h item.h
	gcc $(CFLAGS) -c sudoku-serial.c

list.o: list.c list.h item.h
	gcc $(CFLAGS) -c list.c

clean:
	rm -f *.o *.~ sudoku *.gch
