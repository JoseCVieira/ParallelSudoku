#!/bin/bash

mpicc -g -fopenmp -o sudoku-mpi sudoku-mpi.c list.c
##mpirun -np $1 sudoku-mpi ../sudokus/$2
