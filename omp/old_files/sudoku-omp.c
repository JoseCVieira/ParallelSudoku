#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <omp.h>

// export OMP_NUM_THREADS=2
// http://www.menneske.no/sudoku/eng/random.html?diff=9
// source ~/.profile
// kinst-ompp

#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define ROW(i) i/m_size
#define COL(i) i%m_size

int r_size, m_size, v_size;

int* read_matrix(char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
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

int solve(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int i, fst_val, cp_sudoku[v_size];
    int fst_pos = -1, flag_finish = 0;
    
    // init cp_sudoku
    for(i = 0; i < v_size; i++){
        if(sudoku[i])
            cp_sudoku[i] = UNCHANGEABLE;
        else{
            cp_sudoku[i] = UNASSIGNED;
            if(fst_pos == -1){
                fst_pos = i;
                cp_sudoku[i] = UNCHANGEABLE;
            }
        }
    }
    
    #pragma omp parallel private(i) firstprivate(cp_sudoku)
    {
    int flag_res, val, zeros, i_aux, row, col, val_aux, flag_back, fst_l_val = 0;
    int p_sudoku[v_size], p_r_mask[m_size], p_c_mask[m_size], p_b_mask[m_size];
    
    // a copy for each processor
    for(i = 0; i < v_size; i++){
        p_sudoku[i] = sudoku[i];
        if(i < m_size){
            p_r_mask[i] = rows_mask[i];
            p_c_mask[i] = cols_mask[i];
            p_b_mask[i] = boxes_mask[i];
        }
    }
    
    #pragma omp for nowait
    for(fst_val = 1; fst_val <= m_size; fst_val++){

        flag_res = is_safe_num(p_r_mask, p_c_mask, p_b_mask, fst_pos/m_size, fst_pos%m_size, fst_val);

        if(flag_res){
            p_sudoku[fst_pos] = fst_val;
            
            if(fst_l_val)
                rm_num_masks(fst_l_val, fst_pos/m_size, fst_pos%m_size, p_r_mask, p_c_mask, p_b_mask);

            fst_l_val = fst_val;

            update_masks(fst_val, fst_pos/m_size, fst_pos%m_size, p_r_mask, p_c_mask, p_b_mask);

            zeros = 1;
            flag_back = 0;
            while(zeros){
                zeros = 0;
                
                i = 0;
                if(flag_back){
                    // search nearest element(on their left)
                    for(i = i_aux - 1; i >= 0; i--){
                        if(cp_sudoku[i] > 0 && cp_sudoku[i] <= m_size){
                            row = ROW(i);
                            col = COL(i);
                            
                            val_aux = cp_sudoku[i] + 1;
                            
                            rm_num_masks(cp_sudoku[i], row, col, p_r_mask, p_c_mask, p_b_mask);
                            
                            p_sudoku[i] = UNASSIGNED;
                            
                            if(cp_sudoku[i] < m_size){
                                cp_sudoku[i] = UNASSIGNED;
                                break;
                            }else{
                                cp_sudoku[i] = UNASSIGNED;
                                zeros ++;
                            }
                        }
                    }
                    
                    if(i == -1)
                        break; //impossible
                }
                
                for(i = i; i < v_size; i++){
                    if(!cp_sudoku[i] || flag_back){
                        zeros++;
                        
                        row = ROW(i);
                        col = COL(i);
                        
                        val = 1;
                        if(flag_back){
                            val = val_aux;
                            flag_back = 0;
                        }
                        
                        for(; val <= m_size; val++){
                            
                            flag_res = is_safe_num(p_r_mask, p_c_mask, p_b_mask, row, col, val);
                            
                            if(flag_res){
                                cp_sudoku[i] = val;
                                p_sudoku[i] = val;
                                
                                update_masks(val, row, col, p_r_mask, p_c_mask, p_b_mask);

                                break;
                            }else if(val == m_size){
                                flag_back = 1;
                                i_aux = i;
                                i = v_size; //break
                            }
                        }
                    }
                }
            }
            
            #pragma omp critical
            if(i!=-1 && !flag_finish){
                for(i = 0; i < v_size; i++)
                    sudoku[i] = p_sudoku[i];
                fst_val = m_size + 1;
                flag_finish = 1;
            }
        }
    }

    }
    
    if(flag_finish)
        return 1;
    else
        return 0;
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
