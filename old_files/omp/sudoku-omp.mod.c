#ifdef _POMP
#  undef _POMP
#endif
#define _POMP 200110

#include "sudoku-omp.c.opari.inc"
#line 1 "sudoku-omp.c"
#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// export OMP_NUM_THREADS=2
// http://www.menneske.no/sudoku/eng/random.html?diff=9
// source ~/.profile
// kinst-ompp

#define UNASSIGNED 0

typedef struct{
    int **arr;
} t_array;

int r_size, m_size;

void read_matrix(t_array *grid, char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int **grid, int m_zeros, int* rows_mask, int* cols_mask, int* boxes_mask);
int new_mask( int size);
int int_to_mask(int num);
int init_masks(int** grid, int* rows_mask, int* cols_mask, int* boxes_mask);
void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
int is_safe_num( int* rows_mask, int* cols_mask, int* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);

int main(int argc, char *argv[]) {
    clock_t start, end;
    t_array grid1;
    int result, nr_zeros, i;
    double time;

    if(argc == 2){
        read_matrix(&grid1, argv);

        int* rows_mask = (int*) malloc(m_size * sizeof(int));
        int* cols_mask = (int*) malloc(m_size * sizeof(int));
        int* boxes_mask = (int*) malloc(m_size * sizeof(int));

        printf("\n\ninitial sudoku:");
        print_grid(grid1.arr, r_size, m_size);

        //start measurement
        start = clock();

        nr_zeros = init_masks(grid1.arr, rows_mask, cols_mask, boxes_mask);
        result = solve(grid1.arr, nr_zeros, rows_mask, cols_mask, boxes_mask);

        // end measurement
        end = clock();

        printf("result sudoku:");
        print_grid(grid1.arr, r_size, m_size);
        verify_sudoku(grid1.arr, m_size, r_size) == 1 ? printf("corret, ") : printf("wrong, ");
        result == 1 ? printf("solved!\n") : printf("no solution!\n");

        time = (double) (end-start)/CLOCKS_PER_SEC;
        printf("took %lf sec (%.3lf ms).\n\n", time, time*1000.0);

        free(rows_mask);
        free(cols_mask);
        free(boxes_mask);
        for(i = 0; i < m_size; i++)
            free(grid1.arr[i]);
        free(grid1.arr);
        
    }else
        printf("invalid input arguments.\n");
    
    return 0;
}



int solve(int **grid, int m_zeros, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int i, j, val, max, zeros = 1, flag_back = 0, i_aux, row, col, val_aux;
    int first_cel_pos = -1, first_cell_val, flag_finish = 0, first_cel_row, first_cel_col, first_cel_last_value = 0;
    int vector[m_size * m_size];
    
    // init vector
    for(i = 0; i < m_size; i++){
        for(j = 0; j < m_size; j++){
            if(grid[i][j])
                vector[i * m_size + j] = -1; // unchangeable
            else{
                vector[i * m_size + j] = UNASSIGNED;
                if(first_cel_pos == -1){
                    first_cel_pos = i * m_size + j;
                    first_cel_row = first_cel_pos/m_size;
                    first_cel_col = first_cel_pos%m_size;
                    vector[i * m_size + j] = -1; // unchangeable
                }
            }
        }
    }
    
    max = m_size * m_size;

POMP_Parallel_fork(&omp_rd_5);
#line 102 "sudoku-omp.c"
    #pragma omp parallel     private(i, val, max, zeros, flag_back, i_aux, row, col, val_aux, rows_mask, cols_mask, boxes_mask)
{ POMP_Parallel_begin(&omp_rd_5);
POMP_For_enter(&omp_rd_5);
#line 102 "sudoku-omp.c"
    #pragma omp          for                                                                                                    nowait
    for(first_cell_val = 1; first_cell_val <= m_size; first_cell_val++){
        if(is_safe_num(rows_mask, cols_mask, boxes_mask, first_cel_row, first_cel_col, first_cell_val)){
            
            grid[first_cel_row][first_cel_col] = first_cell_val;
            
            if(first_cel_last_value)
                rm_num_masks(first_cel_last_value, first_cel_row, first_cel_col, rows_mask, cols_mask, boxes_mask);
            
            first_cel_last_value = first_cell_val;
            
            update_masks(first_cell_val, first_cel_row, first_cel_col, rows_mask, cols_mask, boxes_mask);
        }else
            continue;
        
        zeros = 1;
        flag_back = 0;
        while(zeros){
            zeros = 0;
            
            //if(flag_finish)
            //    break;

            i = 0;
            if(flag_back){
                // search nearest element(on their left)
                for(i = i_aux - 1; i >= 0; i--){
                    row = i/m_size;
                    col = i%m_size;
                    
                    if(vector[i] > 0 && vector[i] <= m_size){
                        val_aux = vector[i] + 1;
                        rm_num_masks(vector[i], row, col, rows_mask, cols_mask, boxes_mask);
                        grid[row][col] = UNASSIGNED;
                        
                        if(vector[i] < m_size){
                            vector[i] = UNASSIGNED;
                            break;
                        }else{
                            vector[i] = UNASSIGNED;
                            zeros ++;
                        }
                    }
                }
                
                if(i == -1)
                    break; //impossible
            }
            
            for(i = i; i < max; i++){
                row = i/m_size;
                col = i%m_size;
                    
                if(!vector[i] || flag_back){
                    
                    val = 1;
                    if(flag_back){
                        val = val_aux;
                        flag_back = 0;
                    }
                    zeros++;
                    
                    for(; val <= m_size; val++){
                        if(is_safe_num( rows_mask, cols_mask, boxes_mask, row, col, val)){
                            vector[i] = val;
                            grid[row][col] = val;
                            update_masks(val, row, col, rows_mask, cols_mask, boxes_mask);
                            break;
                        }else if(val == m_size){
                            flag_back = 1;
                            i_aux = i;
                            i = max; //break
                        }
                    }
                }
            }
        }
        print_grid(grid, r_size, m_size);
        //first_cell_val = m_size;
        //flag_finish = 1;
    }
POMP_Barrier_enter(&omp_rd_5);
#pragma omp barrier
POMP_Barrier_exit(&omp_rd_5);
POMP_For_exit(&omp_rd_5);
POMP_Parallel_end(&omp_rd_5); }
POMP_Parallel_join(&omp_rd_5);
#line 183 "sudoku-omp.c"
    return 1;
}

int exists_in( int index, int* mask, int num) {

    int res;
    int masked_num = int_to_mask(num);

    res = mask[index] | masked_num;
    if( res != mask[index]) return 0;

    return 1;
}

int is_safe_num( int* rows_mask, int* cols_mask, int* boxes_mask, int row, int col, int num) {
    return !exists_in( row, rows_mask, num) && !exists_in( col, cols_mask, num) && !exists_in( r_size*(row/r_size)+col/r_size, boxes_mask, num);
}

int new_mask(int size) {
    return (0 << (size-1));
}

int int_to_mask(int num) {
    return (1 << (num-1));
}

int init_masks(int** grid, int* rows_mask, int* cols_mask, int* boxes_mask) {

    int mask, i, row, col, zeros = 0;
    
    //#pragma omp parallel
    //{
    
    //#pragma omp for private(i)
    for(i = 0; i < m_size; i++){
        rows_mask[i]  = UNASSIGNED;
        cols_mask[i]  = UNASSIGNED;
        boxes_mask[i] = UNASSIGNED;
    }

    //#pragma omp for private(row, col, mask) reduction(+:zeros)
    for(row = 0; row < m_size; row++){
        for(col = 0; col < m_size; col++){

            //if the cell has a number add that number to the current row, col and box mask
            if(grid[row][col]){
                mask = int_to_mask( grid[row][col] ); //convert number found to mask ex: if dim=4x4, 3 = 0010
                rows_mask[row] = rows_mask[row] | mask; //to add the new number to the current row's mask use bitwise OR
                
                //#pragma omp atomic
                cols_mask[col] = cols_mask[col] | mask;
                boxes_mask[r_size*(row/r_size)+col/r_size] = boxes_mask[r_size*(row/r_size)+col/r_size] | mask;
            }else
                zeros++;
        }
    }
    //}
    return zeros;
}

void rm_num_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int num_mask = int_to_mask(num);
    rows_mask[row] = rows_mask[row] ^ num_mask;
    cols_mask[col] = cols_mask[col] ^ num_mask;
    boxes_mask[r_size*(row/r_size)+col/r_size] = boxes_mask[r_size*(row/r_size)+col/r_size] ^ num_mask;
}

void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int new_mask = int_to_mask(num);
    rows_mask[row] = rows_mask[row] | new_mask;
    cols_mask[col] = cols_mask[col] | new_mask;
    boxes_mask[r_size*(row/r_size)+col/r_size] = boxes_mask[r_size*(row/r_size)+col/r_size] | new_mask;
}

void read_matrix(t_array *grid, char *argv[]) {
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    int i, j, k, l;
    char *line = NULL, aux[3];

    //verifies if the file was correctly opened
    if ((fp = fopen(argv[1], "r+")) == NULL) {
        fprintf(stderr, "unable to open file %s\n", argv[1]);
        exit(1);
    }

    if( (read = getline(&line, &len, fp)) != -1)
        r_size = atoi(line);

    m_size = r_size *r_size;

    grid->arr = (int **)malloc(m_size * sizeof(int*));

    for(i = 0; i < m_size; i++)
        grid->arr[i] = (int*) malloc(m_size * sizeof(int));

    for(i = 0; getline(&line, &len, fp) != -1; i++){
        k = 0;
        l = 0;
        for (j = 0; line[j] != '\n'; j++) {
            if(line[j] == ' ' || line[j] == '\n'){
                aux[l] = '\0';
                grid->arr[i][k] = atoi(aux);
                l = 0;
                k++;
            }else{
                aux[l] = line[j];
                l++;
            }
        }
    }

    free(line);
    fclose(fp);
}