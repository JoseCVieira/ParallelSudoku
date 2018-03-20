#include <stdio.h>
#include <stdlib.h>

void print_grid(int **grid, int r_size, int m_size) {
    int row, col, aux_1, aux_2;

    printf("\n+");
    for(aux_1 = 0; aux_1 < r_size; aux_1++){
        for(aux_2 = 0; aux_2 < r_size*2+1; aux_2++){
            printf("-");
        }
        printf("+");
    }
    printf("\n");
    
    for (row = 0; row < m_size; row++) {
        if(row % r_size == 0 && row != 0){
            printf("+");
            for(aux_1 = 0; aux_1 < r_size; aux_1++){
                for(aux_2 = 0; aux_2 < r_size*2+1; aux_2++){
                    printf("-");
                }
                printf("+");
            }
            printf("\n|");
        }else
            printf("|");

        for (col = 0; col < m_size; col++){
            if(col % r_size == 0 && col != 0)
                printf(" |");
            printf("%2d", grid[row][col]);
        }
        
        row < m_size-1 ? printf(" |\n") : printf(" |");
    }
    
    printf("\n+");
    for(aux_1 = 0; aux_1 < r_size; aux_1++){
        for(aux_2 = 0; aux_2 < r_size*2+1; aux_2++){
            printf("-");
        }
        printf("+");
    }
    printf("\n\n");
}

char verify_sudoku(int **grid, int m_size){
    int i, j, row, col, num, startRow, startCol;

    for(i = 0; i < m_size; i++){
        for(j = 0; j < m_size; j++){
            
            num = grid[i][j];
            
            if(num != 0){
            
                // row
                for (col = 0; col < 9; col++)
                    if (grid[i][col] == num && row != i && col != j){
                        printf("\nrow=%d | col=%d | num=%d\n", i, j, num);
                        return 0;
                    }
                
                // col
                for (row = 0; row < 9; row++)
                    if (grid[row][j] == num && row != i && col !=j){
                        printf("\nrow=%d | col=%d | num=%d\n", i, j, num);
                        return 0;
                    }
                
                // box
                for (row = 0; row < 3; row++){
                    for (col = 0; col < 3; col++){
                        startRow = i - (i % 3);
                        startCol = j - (j %3);
                        
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
