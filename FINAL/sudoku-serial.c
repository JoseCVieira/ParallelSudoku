#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include "list.h"

#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define ROW(i) i/m_size
#define COL(i) i%m_size

int r_size, m_size, v_size;
int cont = 0;

void update_masks(int num, int row, int col, uint64_t *rows_mask, uint64_t *cols_mask, uint64_t *boxes_mask);
int is_safe_num( uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int solve_from(int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work);
void init_masks(int* sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int exists_in( int index, uint64_t* mask, int num);
int* read_matrix(char *argv[]);
void print_sudoku(int *sudoku);
int int_to_mask(int num);
int new_mask( int size);
int solve(int *sudoku);

int main(int argc, char *argv[]){
    int* sudoku;

    if(argc == 2){
        sudoku = read_matrix(argv);

        if(solve(sudoku))
            print_sudoku(sudoku);
        else
            printf("No solution");
    }else
        printf("invalid input arguments.\n");
    
    free(sudoku);    

    return 0;
}

int solve(int* sudoku){
    int i, flag_start = 0, solved = 0, start_pos = 0, start_num = 0;
    Item hyp;
    
    uint64_t *r_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    uint64_t *c_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    uint64_t *b_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    int *cp_sudokus_array  = (int*)      malloc(v_size * sizeof(int));
    List *list_array = init_list();
    

    for(i = 0; i < v_size; i++) {
        if(sudoku[i])
            cp_sudokus_array[i] = UNCHANGEABLE;
        else{
            cp_sudokus_array[i] = UNASSIGNED;
            if(!flag_start){
                flag_start = !flag_start;
                start_pos = i;
            }
        }
    }
    
    init_masks(sudoku, r_mask_array, c_mask_array, b_mask_array);

    
    //give an initial cell value to each thread
    for(start_num = 1; start_num <= m_size; start_num++) {

        //create first hypothesis and push it to the list
        hyp.cell = start_pos;
        hyp.num = start_num;

        insert_head(list_array, hyp);
        
        if(solve_from(cp_sudokus_array, r_mask_array, c_mask_array, b_mask_array, list_array)) {
            if(!solved){
                solved = 1;
                
                for(i = 0; i < v_size; i++)
                    if(cp_sudokus_array[i] != UNCHANGEABLE)
                        sudoku[i] = cp_sudokus_array[i];
            }
        }
    }

    free(list_array);
    free(r_mask_array);
    free(c_mask_array);
    free(b_mask_array);
    free(cp_sudokus_array);
    
    if(solved)
        return 1;
    return 0;
}

int solve_from(int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work) {
    int cell, val, row, col, zeros = 1, flag_back = 0, cell_aux, first_valid, val_to_insert, next_empty;
    Item hyp;
   
    hyp = pop_head(work);
    
    int start_num = hyp.num;
    int start_pos = hyp.cell;

    if(!is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(start_pos), COL(start_pos), start_num))
        return 0;

    cp_sudoku[start_pos] = start_num;
    update_masks(start_num, ROW(start_pos), COL(start_pos), rows_mask, cols_mask, boxes_mask);
    
    for(cell = start_pos; cell < v_size ; cell++)
        if(cp_sudoku[cell] == UNASSIGNED)
            break;
    next_empty = cell;
    
    while(zeros){
        zeros = 0;

        cell = next_empty;
        if(flag_back){
             flag_back = 0;
            //get next hypothesis to work on from the list
            hyp = pop_head(work);
            
            // put every element back to zero until we get to the new hypothesis cell
            for(cell = cell_aux -1; cell >= next_empty; cell--) {
                if(cp_sudoku[cell] > 0 && cp_sudoku[cell] <= m_size) {
                    
                    rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                    cp_sudoku[cell] = UNASSIGNED;

                    if(cp_sudoku[cell] < m_size){
                        if(cell == hyp.cell){
                            update_masks(hyp.num, ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                            cp_sudoku[cell] = hyp.num;
                            
                            cell = hyp.cell + 1;
                            break;
                        }
                    }
                }
            }
            
            //starting cell's value is incorrect, return
            if(cell == start_pos){
                rm_num_masks(cp_sudoku[start_pos],  ROW(start_pos), COL(start_pos), rows_mask, cols_mask, boxes_mask);
                cp_sudoku[start_pos] = UNASSIGNED;
                return 0;
            }
        }
        
        for(; cell < v_size; cell++){
            
            if(!cp_sudoku[cell]){ //if the cell has a zero
                
                row = ROW(cell);
                col = COL(cell);
                
                zeros++;
                first_valid = 1;
                
                for(val = 1; val <= m_size; val++){
                    if(is_safe_num(rows_mask, cols_mask, boxes_mask, row, col, val)){
                        
                        //explore only the first valid, save its value to insert at the end of the cycle
                        if(first_valid){
                            val_to_insert = val;
                            first_valid = 0;
                        }else{
                            //add the remaining valid ones to work list
                            hyp.cell = cell;
                            hyp.num = val;
                            insert_head(work, hyp);
                        }
                    }
                    if(val == m_size && first_valid == 0){
                        update_masks(val_to_insert, row, col, rows_mask, cols_mask, boxes_mask);
                        cp_sudoku[cell] = val_to_insert;
                        
                        break;
                    }else if(val == m_size){
                        
                        if(work->head == NULL){
                            for(; cell >= start_pos; cell--){
                                if(cp_sudoku[cell] > 0){
                                    rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                                    cp_sudoku[cell] = UNASSIGNED;
                                }
                            }
                            return 0;
                        }
                        
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

int exists_in(int index, uint64_t* mask, int num) {
    int res, masked_num = int_to_mask(num);

    res = mask[index] | masked_num;
    if(res != mask[index]) 
        return 0;
    return 1;
}

int is_safe_num(uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num) {
    return !exists_in(row, rows_mask, num) && !exists_in(col, cols_mask, num) && !exists_in(r_size*(row/r_size)+col/r_size, boxes_mask, num);
}

int new_mask(int size) {
    return (0 << (size-1));
}

int int_to_mask(int num) {
    return (1 << (num-1));
}

void init_masks(int* sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
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

void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
    int num_mask = int_to_mask(num);
    rows_mask[row] = rows_mask[row] ^ num_mask;
    cols_mask[col] = cols_mask[col] ^ num_mask;
    boxes_mask[r_size*(row/r_size)+col/r_size] = boxes_mask[r_size*(row/r_size)+col/r_size] ^ num_mask;
}

void update_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
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
    
    for (i = 0; i < v_size; i++) {
        if(i%m_size != m_size - 1)
            printf("%2d ", sudoku[i]);
        else
            printf("%2d\n", sudoku[i]);
    }
}
