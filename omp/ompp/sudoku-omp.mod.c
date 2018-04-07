#ifdef _POMP
#  undef _POMP
#endif
#define _POMP 200110

#include "sudoku-omp.c.opari.inc"
#line 1 "sudoku-omp.c"
#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "list.h"

#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define ROW(i) i/m_size
#define COL(i) i%m_size

int r_size, m_size, v_size;

int* read_matrix(char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int* sudoku);
int solve_from( int* sudoku, int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask, List* work);
Item get_work(List** list_array);
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

        printf("\n\ninitial sudoku:");
        print_sudoku(sudoku);

        //start measurement
        start = clock();
       
        result = solve(sudoku);

        // end measurement
        end = clock();

        printf("result sudoku:");
        print_sudoku(sudoku);
        
        verify_sudoku(sudoku, r_size) == 1 ? printf("corret, ") : printf("wrong, ");
        result == 1 ? printf("solved!\n") : printf("no solution!\n");

        time = (double) (end-start)/CLOCKS_PER_SEC;
        printf("took %lf sec (%.3lf ms).\n\n", time, time*1000.0);

             
    }else
        printf("invalid input arguments.\n");
    
    free(sudoku);    

    return 0;
}

int solve(int* sudoku){
    
    int ret_val, cell, solved = 0, i;
    int start_pos = 0, start_num = 0;
    int cp_sudoku[v_size], p_sudoku[v_size];
    Item hyp;
    int tnum = omp_get_max_threads();
    List** list_array;
    
    list_array = (List**) malloc(tnum * sizeof(List*));
    for(i = 0; i < tnum; i++) list_array[i] = init_list();
    

    // init cp_sudoku
    for(cell = 0; cell < v_size; cell++){       
        p_sudoku[cell] = sudoku[cell];
        if(sudoku[cell])
            cp_sudoku[cell] = UNCHANGEABLE;
        else
            cp_sudoku[cell] = UNASSIGNED;
    }

POMP_Parallel_fork(&omp_rd_19);
#line 90 "sudoku-omp.c"
    #pragma omp parallel firstprivate(cp_sudoku, p_sudoku, list_array) POMP_DLIST_00019
{ POMP_Parallel_begin(&omp_rd_19);
#line 91 "sudoku-omp.c"
    {
        int j;
        int* r_mask = (int*) malloc(m_size * sizeof(int));
        int* c_mask = (int*) malloc(m_size * sizeof(int));
        int* b_mask = (int*) malloc(m_size * sizeof(int));
        init_masks(p_sudoku, r_mask, c_mask, b_mask);

        
        for( start_pos = 0; start_pos <= v_size; start_pos++)
            if( cp_sudoku[start_pos] == 0) break;

        //give an initial cell value to each thread
POMP_For_enter(&omp_rd_20);
#line 103 "sudoku-omp.c"
        #pragma omp for nowait
        for(start_num = 1; start_num <= m_size; start_num++){

            
            hyp.cell = start_pos; hyp.num = start_num;
            insert_head( list_array[omp_get_thread_num()], hyp);                

            if(solve_from( p_sudoku, cp_sudoku, r_mask, c_mask, b_mask, list_array[omp_get_thread_num()])){ 
POMP_Critical_enter(&omp_rd_21);
#line 111 "sudoku-omp.c"
                #pragma omp critical
{ POMP_Critical_begin(&omp_rd_21);
#line 112 "sudoku-omp.c"
                if(!solved){
                    solved = 1;                    
                    for(j = 0; j < v_size; j++)
                        sudoku[j] = p_sudoku[j];
                }
POMP_Critical_end(&omp_rd_21); }
POMP_Critical_exit(&omp_rd_21);
#line 117 "sudoku-omp.c"
            }
            pop_all(  list_array[omp_get_thread_num()] );
        }
POMP_For_exit(&omp_rd_20);
#line 120 "sudoku-omp.c"

POMP_For_enter(&omp_rd_22);
#line 121 "sudoku-omp.c"
        #pragma omp for nowait
        for(i = 0; i > 0; i++){
            
        
            hyp = get_work(list_array);
            
            if(hyp.num == -1 ) i = -1; //no more work -> break
            else{

                insert_head( list_array[omp_get_thread_num()], hyp);                

                if(solve_from( p_sudoku, cp_sudoku, r_mask, c_mask, b_mask, list_array[omp_get_thread_num()])){ 
POMP_Critical_enter(&omp_rd_23);
#line 133 "sudoku-omp.c"
                    #pragma omp critical
{ POMP_Critical_begin(&omp_rd_23);
#line 134 "sudoku-omp.c"
                    if(!solved){
                        solved = 1;                    
                        for(j = 0; j < v_size; j++)
                            sudoku[j] = p_sudoku[j];

                    }
POMP_Critical_end(&omp_rd_23); }
POMP_Critical_exit(&omp_rd_23);
#line 140 "sudoku-omp.c"
                }
            }
        }
POMP_Barrier_enter(&omp_rd_22);
#pragma omp barrier
POMP_Barrier_exit(&omp_rd_22);
POMP_For_exit(&omp_rd_22);
#line 143 "sudoku-omp.c"

    
        free(r_mask);
        free(c_mask);
        free(b_mask);
    }
POMP_Barrier_enter(&omp_rd_19);
#pragma omp barrier
POMP_Barrier_exit(&omp_rd_19);
POMP_Parallel_end(&omp_rd_19); }
POMP_Parallel_join(&omp_rd_19);
#line 149 "sudoku-omp.c"
    /* FREE DAS LISTAS ============================= */
    if(solved)
        return 1;
    return 0;
}

int solve_from( int* sudoku, int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask, List* work) {
    
    int cell, val, zeros = 1, flag_back = 0, cell_aux, row, col, val_aux;
    Item hyp;
    int first_valid, val_to_insert, aux, counter = 0;
   
    //int tid = omp_get_thread_num();
    //printf("thread: %d , num: %d\n", tid, start_num);
    
    //use the received starting value on the starting cell, create hypothesis from start_cell+1
POMP_Critical_enter(&omp_rd_24);
#line 165 "sudoku-omp.c"
    #pragma omp critical
{ POMP_Critical_begin(&omp_rd_24);
#line 166 "sudoku-omp.c"
    {
        hyp = pop_head(work);
    }
POMP_Critical_end(&omp_rd_24); }
POMP_Critical_exit(&omp_rd_24);
#line 169 "sudoku-omp.c"
    int start_num = hyp.num;
    int start_pos = hyp.cell;

    //if the value given for the first cell isn't valid return
    if(!is_safe_num( rows_mask, cols_mask, boxes_mask, ROW(start_pos), COL(start_pos), start_num)){
        return 0;
    }

    sudoku[start_pos] = start_num;
    cp_sudoku[start_pos] = start_num;
    update_masks(start_num, ROW(start_pos), COL(start_pos), rows_mask, cols_mask, boxes_mask);
    
    while(zeros){
        zeros = 0;

        cell = start_pos+1;;
        if(flag_back){
            flag_back = 0;
             
            //get next hypothesis to work on from the list 
POMP_Critical_enter(&omp_rd_25);
#line 189 "sudoku-omp.c"
            #pragma omp critical
{ POMP_Critical_begin(&omp_rd_25);
#line 190 "sudoku-omp.c"
            { 
                hyp = pop_head(work);
            }
POMP_Critical_end(&omp_rd_25); }
POMP_Critical_exit(&omp_rd_25);
#line 193 "sudoku-omp.c"
            
            // put every element back to zero until we get to the new hypothesis cell
            for(cell = cell_aux; cell >= start_pos+1; cell--){
                if(cp_sudoku[cell] > 0 && cp_sudoku[cell] <= m_size){
                    
                    rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                    sudoku[cell] = UNASSIGNED;
                    cp_sudoku[cell] = UNASSIGNED;

                    if(cell == hyp.cell){
                        update_masks(hyp.num, ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                        cp_sudoku[cell] = hyp.num;
                        sudoku[cell] = hyp.num;
                        counter ++;
                        cell = hyp.cell + 1;
                        break;
                    }
                    if(counter == 50000){
                        //system("clear");
                       // print_sudoku(sudoku);
                        counter = 0;
                    }
                

                }
            }
            
            //starting cell's value is incorrect, return
            if(cell == start_pos - 1){
                rm_num_masks(cp_sudoku[start_pos],  ROW(start_pos), COL(start_pos), rows_mask, cols_mask, boxes_mask);
                sudoku[start_pos] = UNASSIGNED;
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
                        }
                        else {
                            //add the remaining valid ones to work list
                            hyp.cell = cell; hyp.num = val;
POMP_Critical_enter(&omp_rd_26);
#line 250 "sudoku-omp.c"
                            #pragma omp critical
{ POMP_Critical_begin(&omp_rd_26);
#line 251 "sudoku-omp.c"
                            {
                                insert_head( work, hyp);
                            }
POMP_Critical_end(&omp_rd_26); }
POMP_Critical_exit(&omp_rd_26);
#line 254 "sudoku-omp.c"
                        }
                    }
                    if(val == m_size && first_valid == 0){
                            update_masks(val_to_insert, row, col, rows_mask, cols_mask, boxes_mask);
                            cp_sudoku[cell] = val_to_insert;
                            sudoku[cell] = val_to_insert;
                            counter ++;
                            if(counter == 50000){
                                // system("clear")
                               // print_sudoku(sudoku); 
                                counter = 0;
                            }
                            break;
                    }
                    else if(val == m_size){
                        if(work->head == NULL){ 
                            for(;cell>=start_pos;cell--){
                                if(cp_sudoku[cell] > 0){
                                    rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                                    sudoku[cell] = UNASSIGNED;
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

Item get_work(List** list_array){

    int tnum = omp_get_max_threads();
    int th, min_cell = v_size;
    Item hyp;
    hyp.cell = -1; hyp.num = -1;

    for(th = 0; th < tnum; th++){
        if(list_array[th]->tail != NULL){
            if( list_array[th]->tail->this.cell <= min_cell ){
POMP_Critical_enter(&omp_rd_27);
#line 302 "sudoku-omp.c"
                #pragma omp critical
{ POMP_Critical_begin(&omp_rd_27);
#line 303 "sudoku-omp.c"
                {
                    hyp = pop_tail(list_array[th]);
                }
POMP_Critical_end(&omp_rd_27); }
POMP_Critical_exit(&omp_rd_27);
#line 306 "sudoku-omp.c"
            }
        }

    }
    return hyp;
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
