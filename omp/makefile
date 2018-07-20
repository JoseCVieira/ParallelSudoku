CFLAGS= -fopenmp -Wall -Werror

sudoku-omp: sudoku-omp.o list.o
	gcc $(CFLAGS) -o sudoku-omp list.o sudoku-omp.o

sudoku-omp.o: sudoku-omp.c list.h item.h
	gcc $(CFLAGS) -c sudoku-omp.c

list.o: list.c list.h item.h
	gcc $(CFLAGS) -c list.c

clean:
	rm -f *.o *.~ sudoku *.gch
