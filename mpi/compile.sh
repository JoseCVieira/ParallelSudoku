#!/bin/bash

mpicc -g -o sudoku-mpi sudoku-mpi-2nd-approach.c list.c
mpirun -np $1 sudoku-mpi ../sudokus/$2
