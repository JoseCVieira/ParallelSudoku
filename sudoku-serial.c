#include "sudoku-serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UNASSIGNED 0

typedef struct{
    int **arr;
} t_array;

int r_size, m_size;

int read_matrix(t_array *grid, int argc, char *argv[]) {
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    int i, j, k;
    int rooted_matrix_size, n_matrix_size;
    char *line = NULL;

    //verifies if the program calling has at least one argument
    if (argc<1) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        exit(1);
    }
    
    //if it has, opens the file
    if (argc > 1) {
        fprintf(stderr, "opening file %s\n", argv[1]);
        
        //verifies if the file was correctly opened
        if ((fp = fopen(argv[1], "r+")) == NULL) {
            fprintf(stderr, "unable to open file %s\n", argv[1]);
            exit(1);
        }else{
            fprintf(stderr, "%s opened\n",argv[1]);
        }
    }
    
    if(read = getline(&line, &len, fp) != -1)
        rooted_matrix_size = atoi(line);
    
    n_matrix_size = rooted_matrix_size *rooted_matrix_size;

    grid->arr = (int **)malloc(n_matrix_size * sizeof(int*));
    
    for(i = 0; i < n_matrix_size; i++)
        grid->arr[i] = (int*) malloc(n_matrix_size * sizeof(int));

    for(i = 0; getline(&line, &len, fp) != -1; i++){
        k = 0;
        for (j = 0; line[j] != '\n'; j++) {

            if(line[j] == ' '){
                line[j] = '\0';
                k++;
            }else
                grid->arr[i][k] = (int)line[j]-'0';
        }
    }
    
    fclose(fp);
    return rooted_matrix_size;
}

int is_exist_row(int **grid, int row, int num){
    int col;

    for (col = 0; col < 9; col++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

int is_exist_col(int **grid, int col, int num) {
    int row;

    for (row = 0; row < 9; row++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

int is_exist_box(int **grid, int startRow, int startCol, int num) {
    int row, col;

    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            if (grid[row + startRow][col + startCol] == num)
                return 1;
    return 0;
}

int is_safe_num(int **grid, int row, int col, int num) {
    return !is_exist_row(grid, row, num) && !is_exist_col(grid, col, num) && !is_exist_box(grid, row - (row % 3), col - (col %3), num);
}

int find_unassigned(int **grid, int *row, int *col) {
    for (*row = 0; *row < m_size; (*row)++)
        for (*col = 0; *col < m_size; (*col)++)
            if (grid[*row][*col] == 0)
                return 1;
    return 0;
}

int solve(int **grid) {
    int row = 0, col = 0, num;
    double result;
    
    if (!find_unassigned(grid, &row, &col))
        return 1;

    for (num = 1; num <= m_size; num++ ){
        if (is_safe_num(grid, row, col, num)) {
            grid[row][col] = num;

            if (solve(grid))
                return 1;

            grid[row][col] = UNASSIGNED;
        }
    }
        
    return 0;
}

void check_row_box(int **grid, int row, int num, int* v_aux) {
    int i, startRow = row - (row % r_size);

    for(i = startRow; i < (startRow + r_size); i++)
        if(i != row){
            if(is_exist_row(grid, i, num))
                v_aux[i] = i;
            else
                v_aux[i] = 0;
        }else
            v_aux[i] = 0;
}

void check_col_box(int **grid, int col, int num, int* v_aux) {
    int i, startCol = col - (col % r_size);
    
    for(i = startCol; i < (startCol + r_size); i++)
        if(i != col){
            if(is_exist_col(grid, i, num))
                v_aux[i] = i;
            else
                v_aux[i] = 0;
        }else
            v_aux[i] = 0;
}                

void certain_elements(int **grid) {
    int i, j, k, changed = 1, cont;
    int v_aux_c [r_size];
    int v_aux_r [r_size];
    
    while(changed){
        changed = 0;
        
        for(i = 0; i < m_size; i++)
            for(j = 0; j < m_size; j++)
                if(!grid[i][j])
                    for(k = 0; k < m_size; k++){
                        check_row_box(grid, i, k, v_aux_r);
                        check_col_box(grid, j, k, v_aux_c);
                        
                         
                    }
                        
                        
                        
                        /*if(is_safe_num(grid, i, j, k))
                            if(check_row_box(grid, i, k) + check_col_box(grid, j, k) == r_size + 1){
                                grid[i][j] = k;
                                changed = 1;
                                printf("\nchanged => row:%d col:%d, val:%d", i, j, k);
                            }*/
    }
}

/* final print function */

/*void print_grid(int **grid) {
    int row, col;

    for (row = 0; row < m_size; row++) {
        for (col = 0; col < m_size; col++)
            printf("%d ", grid[row][col]);
        printf("\n");
    }
}*/

int main(int argc, char *argv[]) {
    clock_t start, end;
    t_array grid1;
    
    printf("\n");
    r_size = read_matrix(&grid1, argc, argv);
    m_size = r_size * r_size;

    printf("\ninitial sudoku:");
    print_grid(grid1.arr, r_size, m_size);
    printf("solved sudoku:");
    
    //start measurement
    start = clock();
    
    certain_elements(grid1.arr);
    
    // end measurement
    end = clock();
    
    print_grid(grid1.arr, r_size, m_size);
    
    printf("it took %lf sec.\n\n",(double) (end-start)/CLOCKS_PER_SEC);
    
    /*if (solve(grid1.arr)){
        end = clock(); // end measurement
        print_grid(grid1.arr, r_size, m_size);
        
        verify_sudoku(grid1.arr, m_size) == 1 ? printf("\nRigth!\n\n") : printf("\nWrong!\n\n");
    }else{
        
        
        print_grid(grid1.arr, r_size, m_size);
        
        printf("\nNo solution => ");
    }*/
    
    return 0;
}
