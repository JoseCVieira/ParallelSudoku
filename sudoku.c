#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 9
#define UNASSIGNED 0
//TESTE
int is_exist_row(int grid[N][N], int row, int num){
    int col;

    for (col = 0; col < 9; col++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

int is_exist_col(int grid[N][N], int col, int num) {
    int row;

    for (row = 0; row < 9; row++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

int is_exist_box(int grid[N][N], int startRow, int startCol, int num) {
    int row, col;

    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            if (grid[row + startRow][col + startCol] == num)
                return 1;
    return 0;
}

int is_safe_num(int grid[N][N], int row, int col, int num) {
    return !is_exist_row(grid, row, num) && !is_exist_col(grid, col, num) && !is_exist_box(grid, row - (row % 3), col - (col %3), num);
}

int find_unassigned(int grid[N][N], int *row, int *col) {
    for (*row = 0; *row < N; (*row)++)
        for (*col = 0; *col < N; (*col)++)
            if (grid[*row][*col] == 0)
                return 1;
    return 0;
}

int solve(int grid[N][N]) {

    double result;

    int row = 0, col = 0, num;

    if (!find_unassigned(grid, &row, &col))
        return 1;

    for (num = 1; num <= N; num++ )
        if (is_safe_num(grid, row, col, num)) {
            grid[row][col] = num;

            if (solve(grid))
                return 1;

            grid[row][col] = UNASSIGNED;
        }


    return 0;
}

void print_grid(int grid[N][N]) {
    int row, col;

    printf("\n+-------+-------+-------+\n");
    for (row = 0; row < N; row++) {
        if(row == 3 || row == 6)
            printf("+-------+-------+-------+\n|");
        else
            printf("|");

        for (col = 0; col < N; col++){
            if(col == 3 || col == 6)
                printf(" |");
            printf("%2d", grid[row][col]);
        }
        printf(" |");
        printf("\n");
    }
    printf("+-------+-------+-------+\n\n");
}

int main() {
    int grid[N][N] = {{4,0,0, 0,0,1, 0,0,5},
                      {0,0,1, 0,3,0, 0,0,0},
                      {3,0,0, 2,9,0, 0,0,0},
                      {0,9,3, 0,0,7, 8,4,0},
                      {0,0,0, 0,5,0, 0,7,0},
                      {0,6,0, 1,0,0, 0,0,0},
                      {0,8,5, 0,0,0, 9,0,0},
                      {7,0,0, 5,0,0, 0,0,0},
                      {0,0,0, 0,4,2, 0,0,0}};

    clock_t start, end;
    start = clock();
    if (solve(grid)){
        end = clock();
        print_grid(grid);
    }else{
        end = clock();
        printf("no solution");
      }

    printf("%f",(double) (end-start)/CLOCKS_PER_SEC);
    return 0;
}
