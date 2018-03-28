#include <stdio.h>
#include <stdlib.h>

void print_grid(int **grid, int r_size, int m_size) {
    int row, col;

    printf("\n");
    
    for (row = 0; row < m_size; row++) {
        if(row % r_size == 0 && row != 0){
            printf("\n|");
        }else
            printf("|");

        for (col = 0; col < m_size; col++){
            if(col % r_size == 0 && col != 0)
                printf(" |");
            printf("%2d ", grid[row][col]);
        }
        
        row < m_size-1 ? printf(" |\n") : printf(" |");
    }
    printf("\n\n");
}

int verify_sudoku(int **grid, int m_size, int r_size){
    int i=0, j=0, row=0, col, num, startRow, startCol;

    for(i = 0; i < m_size; i++){
        for(j = 0; j < m_size; j++){
            
            num = grid[i][j];
            
            if(num != 0){
            
                // row
                for (col = 0; col < m_size; col++)
                    if (grid[i][col] == num && row != i && col != j){
                        printf("\nrow=%d | col=%d | num=%d\n", i, j, num);
                        return 0;
                    }
                
                // col
                for (row = 0; row < m_size; row++)
                    if (grid[row][j] == num && row != i && col !=j){
                        printf("\nrow=%d | col=%d | num=%d\n", i, j, num);
                        return 0;
                    }
                
                // box
                for (row = 0; row < r_size; row++){
                    for (col = 0; col < r_size; col++){
                        startRow = i - (i % r_size);
                        startCol = j - (j % r_size);
                        
                        if (grid[row + startRow][col + startCol] == num  && (row + startRow) != i && (col + startCol) != j){
                            printf("\nrow=%d | col=%d | num=%d\n", i, j, num);
                            return 0;
                        }
                    }
                }
            }
        }
    }
    
    return 1;
}
