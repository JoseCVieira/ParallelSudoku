CFLAGS= -fopenmp

sudoku-mpi:
        mpicc $(CFLAGS) -o sudoku-mpi list.c sudoku-mpi.c
sudoku-serial:
        gcc -o sudoku-serial sudoku-serial.c list.c
clean:
        rm -f *.o *.~ sudoku *.gch