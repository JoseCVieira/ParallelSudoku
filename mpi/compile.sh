#!/bin/bash

mpicc -g -o sudoku-mpi sudoku-mpi-v3.c list.c
mpirun -np $1 sudoku-mpi ../sudokus/$2
