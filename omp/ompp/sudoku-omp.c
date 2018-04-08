#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include "list.h"

#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define ROW(i) i/m_size
#define COL(i) i%m_size

int r_size, m_size, v_size;

int* read_matrix(char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int* sudoku);
int solve_from( int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask, List* work);
Item get_work( int tid, int **cp_sudokus_array, List** list_array, int **r_mask_array, int **cols_mask_array, int **b_mask_array);
void clear_all_work( int* cp_sudoku, List* work, int* rows_mask, int* cols_mask, int* boxes_mask);
int termination_test( int* terminated);
int new_mask( int size);
int int_to_mask(int num);
void init_masks(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
void update_masks(int num, int row, int col, int *rows_mask, int *cols_mask, int *boxes_mask);
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
    
    int ret_val, cell, solved = 0, i, j;
    int start_pos = 0, start_num = 0;
    int cp_sudoku[v_size];
    Item hyp;
    int tnum = omp_get_max_threads();

    //array with indication of termination of the parallell for
    int *terminated = (int*) malloc(tnum * sizeof(int));
    for(i = 0; i < tnum ; i++) terminated[i] = 0;

    //init array of work lists, one for each thread
    List **list_array = (List**) malloc(tnum * sizeof(List*));
    for(i = 0; i < tnum; i++) list_array[i] = init_list();

    //init array of masks, one for each thread
    int **r_mask_array = (int**)malloc(tnum * sizeof(int*));
    int **c_mask_array = (int**)malloc(tnum * sizeof(int*));
    int **b_mask_array = (int**)malloc(tnum * sizeof(int*));
    for(i = 0; i < tnum; i++){
        r_mask_array[i] = (int*) malloc(m_size * sizeof(int));
        c_mask_array[i] = (int*) malloc(m_size * sizeof(int));
        b_mask_array[i] = (int*) malloc(m_size * sizeof(int));
        init_masks(sudoku, r_mask_array[i], c_mask_array[i], b_mask_array[i]);
    }

    // init cp_sudoku
    for(cell = 0; cell < v_size; cell++){       
        if(sudoku[cell])
            cp_sudoku[cell] = UNCHANGEABLE;
        else
            cp_sudoku[cell] = UNASSIGNED;
    }

    //init array of cp_sudokus, one for each thread
    int **cp_sudokus_array = (int**) malloc(tnum * sizeof(int*));
    
    for( i = 0; i < tnum; i++){
        cp_sudokus_array[i] = (int*)malloc(v_size*sizeof(int));
        for(j = 0; j < v_size; j++){
            cp_sudokus_array[i][j] = cp_sudoku[j];
        }
    }

    //find first position with a zero
    for( start_pos = 0; start_pos <= v_size; start_pos++)
            if( cp_sudoku[start_pos] == 0) break;

    #pragma omp parallel if( m_size > 4 ) private(hyp, i)
    {
        int j;
        int tid = omp_get_thread_num();

        //give an initial cell value to each thread
        #pragma omp for nowait schedule( dynamic, 1)     
        for(start_num = 1; start_num <= m_size; start_num++){

            hyp.cell = start_pos; hyp.num = start_num; //create first hypothesis and push it to the list
            #pragma omp critical(lists)
            {    
                insert_head( list_array[tid], hyp);                
            }

            printf("thread %d gets start_num %d\n", tid, start_num );
            printf(" THREAD: %d - ", omp_get_thread_num()); print_list(list_array[tid]); printf("\n");
            if(solve_from( cp_sudokus_array[tid], r_mask_array[tid], c_mask_array[tid], b_mask_array[tid], list_array[tid])){ 
                #pragma omp critical(sudoku)
                if(!solved){
                    solved = 1;                    
                    for(j = 0; j < v_size; j++){
                        if(cp_sudokus_array[tid][j] != UNCHANGEABLE){
                            sudoku[j] = cp_sudokus_array[tid][j];
                        }
                    }
                }
            }
            else{
                clear_all_work( cp_sudokus_array[tid], list_array[tid], r_mask_array[tid], c_mask_array[tid], b_mask_array[tid] );
                terminated[tid] = 1;
            }
            printf("thread %d finished start_cell: %d , start_num: %d\n", tid, start_pos, start_num);
        }

        for(i = 0; i >= 0;){
          
            if(solved) i = -2; //is solve=1 break, else continue
            else{
                //get work from the other threads
                hyp = get_work( tid, cp_sudokus_array, list_array, r_mask_array, c_mask_array, b_mask_array);
                
                if(hyp.num == -1 ){
                    if(termination_test(terminated) )i = -2; //no more work verse já todas as threads terminaram a o for inicial se sim exit, se não  try again?
                }
                else{
                    #pragma omp critical(lists)
                    {
                        insert_head( list_array[tid], hyp);  //push the starting hypothesis received to the work list            
                    }
                    print_list(list_array[tid]); printf(" THREAD: %d\n", omp_get_thread_num());
                    if(solve_from( cp_sudokus_array[tid], r_mask_array[tid], c_mask_array[tid], b_mask_array[tid], list_array[tid])){ 
                        #pragma omp critical(sudoku)
                        if(!solved){
                            solved = 1;                    
                            for(j = 0; j < v_size; j++){
                                if(cp_sudokus_array[tid][j] != UNCHANGEABLE){
                                    sudoku[j] = cp_sudokus_array[tid][j];
                                }
                            }
                        }
                    }
                    else //Não sei se isto aqui é necessário
                        clear_all_work( cp_sudokus_array[tid], list_array[tid], r_mask_array[tid], c_mask_array[tid], b_mask_array[tid] );
                }
            }
        }
    }
    /* FREE DAS LISTAS, MASKS, SUDOKUS ============================= */
    if(solved)
        return 1;
    return 0;
}

int solve_from( int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask, List* work) {
    
    int cell, val, zeros = 1, flag_back = 0, cell_aux, row, col, val_aux;
    Item hyp;
    int first_valid, val_to_insert, aux, counter = 0;
    
    //use the received starting value on the starting cell, create hypothesis from start_cell+1
    #pragma omp critical(lists)
    {
        hyp = pop_head(work);
    }
    int start_num = hyp.num;
    int start_pos = hyp.cell;

    //if the value given for the first cell isn't valid return
    if(!is_safe_num( rows_mask, cols_mask, boxes_mask, ROW(start_pos), COL(start_pos), start_num)){
        return 0;
    }

    cp_sudoku[start_pos] = start_num;
    update_masks(start_num, ROW(start_pos), COL(start_pos), rows_mask, cols_mask, boxes_mask);
    
    while(zeros){
        zeros = 0;

        cell = start_pos+1;;
        if(flag_back){
            flag_back = 0;
             
            //get next hypothesis to work on from the list 
            #pragma omp critical(lists)
            { 
                hyp = pop_head(work);
               // if(omp_get_thread_num() == 0){ print_list(work); printf("\n"); }
            }
            
            // put every element back to zero until we get to the new hypothesis cell
            for(cell = cell_aux; cell >= start_pos+1; cell--){
                if(cp_sudoku[cell] > 0 && cp_sudoku[cell] <= m_size){
                    
                    rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                    cp_sudoku[cell] = UNASSIGNED;

                    if(cell == hyp.cell){
                        update_masks(hyp.num, ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                        cp_sudoku[cell] = hyp.num;
                        cell = hyp.cell + 1;
                        break;
                    }
                }
            }
            
            //starting cell's value is incorrect, return
            if(cell == start_pos - 1){
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
                        }
                        else {
                            //add the remaining valid ones to work list
                            hyp.cell = cell; hyp.num = val;
                            #pragma omp critical(lists)
                            {
                                insert_head( work, hyp);
                            //    if(omp_get_thread_num() == 0){ print_list(work); printf("\n"); }
        
                            }
                        }
                    }
                    if(val == m_size && first_valid == 0){
                            update_masks(val_to_insert, row, col, rows_mask, cols_mask, boxes_mask);
                            cp_sudoku[cell] = val_to_insert;
                          
                            break;
                    }
                    else if(val == m_size){
                        if(work->head == NULL){ 
                            for(;cell>=start_pos;cell--){
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

Item get_work( int tid, int **cp_sudokus_array, List** list_array, int **r_mask_array, int **c_mask_array, int **b_mask_array){

    int tnum = omp_get_max_threads();
    int th, min_cell = v_size, i;
    int receive_from_th = -1;
    Item hyp;
    
    hyp.cell = -1; hyp.num = -1;
    
        for(th = 0; th < tnum; th++){
            if(list_array[th]->tail != NULL){ //&& list_array[th]->head->next != NULL){
                if( list_array[th]->tail->this.cell <= min_cell ){
                    min_cell = list_array[th]->tail->this.cell;
                    receive_from_th = th;
                }
            }
        }
        if(receive_from_th != -1){
            #pragma omp critical(lists)
                    {   
                        hyp = pop_tail(list_array[receive_from_th]);
                       // if(omp_get_thread_num() == 0){ print_list(list_array[th]); printf("\n"); }
                       
                    }
        
            //receber o sudoku e máscaras da thread que está a dar o trabalho;
            for( i = 0; i < v_size; i++){
                cp_sudokus_array[tid][i] = cp_sudokus_array[receive_from_th][i];
            }
            for( i = 0; i < m_size; i++){
                r_mask_array[tid][i] = r_mask_array[receive_from_th][i];
                c_mask_array[tid][i] = c_mask_array[receive_from_th][i];
                b_mask_array[tid][i] = b_mask_array[receive_from_th][i];
            }
            // apagar tudo o que esta fez até á posição hyp.cell
            i = v_size;
            while(i >= hyp.cell){
                if(cp_sudokus_array[tid][i] > 0){
                    rm_num_masks( cp_sudokus_array[tid][i],  ROW(i), COL(i), r_mask_array[tid], c_mask_array[tid], b_mask_array[tid]);
                    cp_sudokus_array[tid][i] = UNASSIGNED;
                }
                i--;
            }


            printf("\nThread %d: receive (%d,%d), list size: %d\n", tid, hyp.cell, hyp.num, listSize(list_array[tid]) );
            if(list_array[receive_from_th]->tail == NULL) printf("Thread %d: tail is now NULL, list size: %d\n", receive_from_th, listSize(list_array[receive_from_th]));
            else{ printf("Thread %d: tail is now: (%d,%d), list size: %d\n", receive_from_th, list_array[receive_from_th]->tail->this.cell, list_array[receive_from_th]->tail->this.num, listSize(list_array[receive_from_th]));
            /*if(receive_from_th == 0){ printf("Th0 list:"); print_list(list_array[receive_from_th]); printf("\n");}*/}
           // print_sudoku(cp_sudokus_array[tid]);
            printf("\n"); 
        }
    return hyp;
}

void clear_all_work( int *cp_sudoku, List *work, int *rows_mask, int *cols_mask, int *boxes_mask){
    int i;
    
    for(i = 0; i < v_size; i++){
        if(cp_sudoku[i] != UNCHANGEABLE){
            rm_num_masks(cp_sudoku[i],  ROW(i), COL(i), rows_mask, cols_mask, boxes_mask);
            cp_sudoku[i] = UNASSIGNED;
        }
    }
    pop_all(  work );

    return;
}

int termination_test( int* terminated){
    int i;
    int tnum = omp_get_max_threads();

    for(i = 0; i < tnum; i++){
        if(terminated[i] == 0 ) return 0;
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
