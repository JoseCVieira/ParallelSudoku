#include <stdio.h>
#include <stdlib.h>

int verify_sudoku(int *sudoku, int r_size){
    int i, row, col, num, startRow, startCol;
    int m_size = r_size * r_size;
    int v_size = m_size * m_size;

    for(i = 0; i < v_size; i++){
        num = sudoku[i];
        
        if(num != 0){
            // row
            for (col = 0; col < m_size; col++)
                if (sudoku[(i/m_size)*m_size + col] == num && col != i%m_size)
                    return 0;
            
            // col
            for (row = 0; row < m_size; row++)
                if (sudoku[row*m_size + (i%m_size)] == num && row != i/m_size)
                    return 0;
            
            // box
            for (row = 0; row < r_size; row++){
                for (col = 0; col < r_size; col++){
                    startRow = (i/m_size) - ((i/m_size) % r_size);
                    startCol = (i%m_size) - ((i%m_size) % r_size);
                    
                    if (sudoku[(row + startRow)*m_size + col + startCol] == num  && (row + startRow) != i/m_size && (col + startCol) != i%m_size)
                        return 0;
                }
            }
        }
    }
    
    return 1;
}
