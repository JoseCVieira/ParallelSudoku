#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define ROW(i) i/m_size
#define COL(i) i%m_size

int r_size, m_size, v_size;

int* read_matrix(char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
int solve_from( int start_pos, int start_num, int* sudoku, int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
int new_mask( int size);
int int_to_mask(int num);
void init_masks(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
int is_safe_num( int* rows_mask, int* cols_mask, int* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
void print_sudoku(int* sudoku);

int main(int argc, char *argv[]) {
    clock_t start, end;
    int result;
    double time;
    int* sudoku;

    if(argc == 2){
        sudoku = read_matrix(argv);

        int* rows_mask = (int*) malloc(m_size * sizeof(int));
        int* cols_mask = (int*) malloc(m_size * sizeof(int));
        int* boxes_mask = (int*) malloc(m_size * sizeof(int));

        printf("\n\ninitial sudoku:");
        print_sudoku(sudoku);

        //start measurement
        start = clock();

        init_masks(sudoku, rows_mask, cols_mask, boxes_mask);        
        result = solve(sudoku, rows_mask, cols_mask, boxes_mask);

        // end measurement
        end = clock();

        printf("result sudoku:");
        print_sudoku(sudoku);
        
        verify_sudoku(sudoku, r_size) == 1 ? printf("corret, ") : printf("wrong, ");
        result == 1 ? printf("solved!\n") : printf("no solution!\n");

        time = (double) (end-start)/CLOCKS_PER_SEC;
        printf("took %lf sec (%.3lf ms).\n\n", time, time*1000.0);

        free(rows_mask);
        free(cols_mask);
        free(boxes_mask);
        free(sudoku);        
    }else
        printf("invalid input arguments.\n");
    
    return 0;
}

int solve(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask){
    
    int ret_val, cell, solved = 0;
    int start_pos = 0, start_num = 0;
    int cp_sudoku[v_size], p_sudoku[v_size];

    // init cp_sudoku
    for(cell = 0; cell < v_size; cell++){       
        p_sudoku[cell] = sudoku[cell];
        if(sudoku[cell])
            cp_sudoku[cell] = UNCHANGEABLE;
        else
            cp_sudoku[cell] = UNASSIGNED;
    }

    #pragma omp parallel firstprivate(cp_sudoku, p_sudoku)
    {
        int j;
        int* r_mask = (int*) malloc(m_size * sizeof(int));
        int* c_mask = (int*) malloc(m_size * sizeof(int));
        int* b_mask = (int*) malloc(m_size * sizeof(int));
        init_masks(sudoku, r_mask, c_mask, b_mask);

        #pragma omp for
        for(start_num = 1; start_num <= m_size; start_num++){
            if(!solved){
                if(solve_from(start_pos, start_num, p_sudoku, cp_sudoku, r_mask, c_mask, b_mask)){
                    #pragma omp critical
                    if(!solved){
                        solved = 1;                    
                        for(j = 0; j < v_size; j++)
                            sudoku[j] = p_sudoku[j];
                    }
                }
            }
        }
    }
  
    if(solved)
        return 1;
    return 0;
}

int solve_from(int start_pos, int start_num, int* sudoku, int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask) {
    
    int cell, val, zeros = 1, flag_back = 0, cell_aux, row, col, val_aux;
    
    //if the value given for the first cell isn't valid return
    if(!is_safe_num( rows_mask, cols_mask, boxes_mask, ROW(start_pos), COL(start_pos), start_num))
        return 0;
    
    //use the received starting value on the starting cell, create hypothesis from start_cell+1
    sudoku[start_pos] = start_num;
    cp_sudoku[start_pos] = start_num;
    update_masks(start_num, ROW(start_pos), COL(start_pos), rows_mask, cols_mask, boxes_mask);
    
    while(zeros){
        zeros = 0;

        cell = start_pos;
        if(flag_back){
            // search nearest element(on their left)
            for(cell = cell_aux - 1; cell >= start_pos; cell--){
                if(cp_sudoku[cell] > 0 && cp_sudoku[cell] <= m_size){
                    
                    val_aux = cp_sudoku[cell] + 1;
                    rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                    sudoku[cell] = UNASSIGNED;
                    
                    if(cp_sudoku[cell] < m_size){
                        cp_sudoku[cell] = UNASSIGNED;
                        break;
                    }else{
                        cp_sudoku[cell] = UNASSIGNED;
                        zeros ++;
                    }
                }
            }
            
            //starting cell's value is incorrect, return
            if(cell == start_pos - 1)
                return 0;
        }
        
        for(; cell < v_size; cell++){
            if(!cp_sudoku[cell]){
                row = ROW(cell);
                col = COL(cell);
                
                val = 1;
                if(flag_back){
                    val = val_aux;
                    flag_back = 0;
                }
                
                zeros++;
                
                for(; val <= m_size; val++){
                    if(is_safe_num(rows_mask, cols_mask, boxes_mask, row, col, val)){
                        update_masks(val, row, col, rows_mask, cols_mask, boxes_mask);
                        cp_sudoku[cell] = val;
                        sudoku[cell] = val;
                        break;
                    }else if(val == m_size){
                        flag_back = 1;
                        cell_aux = cell;
                        cell = v_size; //break
                    }
                }
            }
        }
    }
    return 1;
}

int exists_in(int index, int* mask, int num) {
    int res, masked_num = int_to_mask(num);

    res = mask[index] | masked_num;
    if(res != mask[index]) 
        return 0;
    return 1;
}

int is_safe_num(int* rows_mask, int* cols_mask, int* boxes_mask, int row, int col, int num) {
    return !exists_in(row, rows_mask, num) && !exists_in(col, cols_mask, num) && !exists_in(r_size*(row/r_size)+col/r_size, boxes_mask, num);
}

int new_mask(int size) {
    return (0 << (size-1));
}

int int_to_mask(int num) {
    return (1 << (num-1));
}

void init_masks(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int i, mask, row, col;

    for(i = 0; i < m_size; i++){
        rows_mask[i]  = UNASSIGNED;
        cols_mask[i]  = UNASSIGNED;
        boxes_mask[i] = UNASSIGNED;
    }

    for(i = 0; i < v_size; i++){
        if(sudoku[i]){
            row = ROW(i);
            col = COL(i);
            
            mask = int_to_mask(sudoku[i]);          //convert number found to mask ex: if dim=4x4, 3 = 0010
            rows_mask[row] = rows_mask[row] | mask; //to add the new number to the current row's mask use bitwise OR
            cols_mask[col] = cols_mask[col] | mask;
            boxes_mask[r_size*(row/r_size)+col/r_size] = boxes_mask[r_size*(row/r_size)+col/r_size] | mask;
        }
    }
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

int* read_matrix(char *argv[]) {
    FILE *fp;
    size_t characters, len = 1;
    char *line = NULL, aux[3];
    int i, j, k, l;
    
    //verifies if the file was correctly opened
    if((fp = fopen(argv[1], "r+")) == NULL) {
        fprintf(stderr, "unable to open file %s\n", argv[1]);
        exit(1);
    }

    getline(&line, &len, fp);
    r_size = atoi(line);    
    m_size = r_size *r_size;
    v_size = m_size * m_size;

    int* sudoku = (int*)malloc(v_size * sizeof(int));

    k = 0, l = 0;
    len = m_size * 2;
    for(i = 0; (characters = getline(&line, &len, fp)) != -1; i++){
        for (j = 0; j < characters; j++) {
            if(isdigit(line[j])){
                aux[l++] = line[j];
            }else if(l > 0){
                aux[l] = '\0';
                l = 0;
                sudoku[k++] = atoi(aux);
                memset(aux, 0, sizeof aux);
            }
        }
    }

    free(line);
    fclose(fp);
    
    return sudoku;
}

void print_sudoku(int *sudoku) {
    int i;
    
    printf("\n\n");
    for (i = 0; i < v_size; i++) {
        if(i%m_size != m_size - 1){
            if(i%m_size%r_size == 0 && i%m_size != 0)
                printf("|");
            printf("%2d ", sudoku[i]);
        }else{
            printf("%2d\n", sudoku[i]);
            if(((i/m_size)+1)%r_size == 0)
                printf("\n");
            
        }
    }
    printf("\n\n");
}
