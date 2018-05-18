#!/bin/bash

mpicc -g -fopenmp -o sudoku-mpi sudoku-mpi.c list.c

if [ "$1" -eq 0 ]; then
    time ./sudoku-serial ../sudokus/$2
elif [ "$1" -eq 1 ]; then
    mpirun --host cpd-1,cpd-2 sudoku-mpi ../sudokus/$2
elif [ "$1" -eq 2 ]; then
    mpirun --host cpd-1,cpd-2,cpd-3 sudoku-mpi ../sudokus/$2
elif [ "$1" -eq 3 ]; then
    mpirun --host cpd-1,cpd-2,cpd-3,cpd-4 sudoku-mpi ../sudokus/$2
elif [ "$1" -eq 4 ]; then
    mpirun --host cpd-1,cpd-2,cpd-3,cpd-4,cpd-5 sudoku-mpi ../sudokus/$2
elif [ "$1" -eq 5 ]; then
    mpirun --host cpd-1,cpd-2,cpd-3,cpd-4,cpd-5,cpd-6 sudoku-mpi ../sudokus/$2
elif [ "$1" -eq 6 ]; then
    mpirun --host cpd-1,cpd-2,cpd-3,cpd-4,cpd-5,cpd-6,cpd-7 sudoku-mpi ../sudokus/$2
elif [ "$1" -eq 7 ]; then
    mpirun --host cpd-1,cpd-2,cpd-3,cpd-4,cpd-5,cpd-6,cpd-7,cpd-8 sudoku-mpi ../sudokus/$2
elif [ "$1" -eq 8 ]; then
    mpirun --host cpd-1,cpd-2,cpd-3,cpd-4,cpd-5,cpd-6,cpd-7,cpd-8,cpd-9 sudoku-mpi ../sudokus/$2
##mpirun -np $1 sudoku-mpi ../sudokus/$2
