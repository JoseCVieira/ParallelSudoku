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
int cont = 0;

int* read_matrix(char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int* sudoku);
int solve_from(int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask, List* work);
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

        start = clock();
        result = solve(sudoku);
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

int solve(int* sudoku) {
    int i, j, flag_start = 0, solved = 0, start_pos = 0, start_num = 0, tnum = omp_get_max_threads();
    Item hyp;
    
    /* Initializations */
    List **list_array      = (List**) malloc(tnum * sizeof(List*));
    int *terminated        = (int*)   malloc(tnum * sizeof(int));
    int **r_mask_array     = (int**)  malloc(tnum * sizeof(int*));
    int **c_mask_array     = (int**)  malloc(tnum * sizeof(int*));
    int **b_mask_array     = (int**)  malloc(tnum * sizeof(int*));
    int **cp_sudokus_array = (int**)  malloc(tnum * sizeof(int*));
    

    for(i = 0; i < tnum; i++){
        
        //init array of work lists, one for each thread. 
        // A work list contains the pairs (cell_id, number) from wich serial DFS search must still be performed 
        //(in other words it cointains the root values of the unexplored parts of the search tree)
        list_array[i] = init_list();

        //init array of masks, one for each thread
        r_mask_array[i]     = (int*) malloc(m_size * sizeof(int));
        c_mask_array[i]     = (int*) malloc(m_size * sizeof(int));
        b_mask_array[i]     = (int*) malloc(m_size * sizeof(int));
        cp_sudokus_array[i] = (int*) malloc(v_size * sizeof(int));
        
        //init array of work lists, one for each thread
        init_masks(sudoku, r_mask_array[i], c_mask_array[i], b_mask_array[i]);
        
        //init array of cp_sudokus, one for each thread
        for(j = 0; j < v_size; j++) {
            if(sudoku[j])
                cp_sudokus_array[i][j] = UNCHANGEABLE;
            else{
                cp_sudokus_array[i][j] = UNASSIGNED;
                if(!flag_start){
                    flag_start = !flag_start;
                    start_pos = j;
                }
            }
        }

        //array with indication of termination of the parallell for, one cell for each thread (if terminated the parallel for set to 1)
        terminated[i] = 0;
    }
    

    #pragma omp parallel private(hyp, i) shared(solved, start_pos, start_num, sudoku, list_array, r_mask_array, c_mask_array, b_mask_array, cp_sudokus_array)
    {
        int tid = omp_get_thread_num();
        
        // Parallell Stage 1
        // Each thread starts with a different initial value for the first free cell; 
        // When a thread finishes searching the search tree for it's initial value, it tries with a different value (dynamic scheduling)
        // If there are no more initial values to search the thread continues to the Parallell Stage 2
        #pragma omp for nowait schedule(dynamic, 1)
        for(start_num = 1; start_num <= m_size; start_num++) {

            //create first hypothesis
            hyp.cell = start_pos;
            hyp.num = start_num;

            //push first hypothesis it to the work list
            #pragma omp critical(hyp)
            insert_head(list_array[tid], hyp);
            
            //do serial DFS starting from the cell and number indicated in the first hypothesis
            if(solve_from(cp_sudokus_array[tid], r_mask_array[tid], c_mask_array[tid], b_mask_array[tid], list_array[tid])) {
                //if the solution is found set flag solved to 1 and copy the solution to be retrieved
                #pragma omp critical(sudoku)
                if(!solved){
                    solved = 1;                        
                    for(i = 0; i < v_size; i++)
                        if(cp_sudokus_array[tid][i] != UNCHANGEABLE)
                            sudoku[i] = cp_sudokus_array[tid][i];
                }
            //if no solution is found clear all the work done from the sudoku and from the masks; go to Parrallell Stage 2
            }else{    
                clear_all_work(cp_sudokus_array[tid], list_array[tid], r_mask_array[tid], c_mask_array[tid], b_mask_array[tid]);
                terminated[tid] = 1;
            }
        }

        //Parallell Stage 2
        //Each thread searches the work lists of the other threads, trying to find work to do. 
        //When a node is found the thread removes it from the other thread's list allong with the state of the masks and the sudoku
        // places the node in it's own list and starts Serial DFS using that node as root
        //if no solution is found, the thread searches the works lists again to find more work
        for(i = 0; i >= 0;){ //infinite loop
            if(solved)
                i = -2; //is solve=1 break, else continue
            else{

                //get work from the other threads: a node to start DFS is received and the sudoku and masks are set to that node.
                //If no work is found a node initialized to (-1,-1) is retrieved
                #pragma omp critical(hyp)
                hyp = get_work(tid, cp_sudokus_array, list_array, r_mask_array, c_mask_array, b_mask_array);
                
                //no work was found
                if(hyp.num == -1){
                    //do a termination test to see if there is still any thread in Parallell Stage 1. If no thread is there anymore the Sudoku has no solution
                    //if there are still threads in Parallell Stage 2 keep trying to find more work
                    if(termination_test(terminated) )i = -2;
                //work was found
                }else{
                    //push the received node to the work list
                    #pragma omp critical(hyp)
                    insert_head(list_array[tid], hyp);           
                    
                    //do Serial DFS from the starting node received
                    if(solve_from( cp_sudokus_array[tid], r_mask_array[tid], c_mask_array[tid], b_mask_array[tid], list_array[tid])){ 
                        #pragma omp critical(sudoku)
                        if(!solved){
                            solved = 1;                  
                            for(j = 0; j < v_size; j++)
                                if(cp_sudokus_array[tid][j] != UNCHANGEABLE)
                                    sudoku[j] = cp_sudokus_array[tid][j];              
                        }
                    }
                    //if no solution is found clear the sudoku and the masks, go find more work
                    else
                        clear_all_work( cp_sudokus_array[tid], list_array[tid], r_mask_array[tid], c_mask_array[tid], b_mask_array[tid] );
                }
            }
        }
    }
    
    for(i = 0; i < tnum; i++){
        free(list_array[i]);
        free(r_mask_array[i]);
        free(c_mask_array[i]);
        free(b_mask_array[i]);
        free(cp_sudokus_array[i]);
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

//Perform Depth First Search from an initial condition of (cell_id, num)
int solve_from(int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask, List* work) {
    int cell, val, row, col, zeros = 1, flag_back = 0, cell_aux, first_valid, val_to_insert, next_empty;
    Item hyp;
    
    //Get the initial condition, which contains the starting cell to solve in serial mode from, and the number to put in that cell
    #pragma omp critical(hyp)
    hyp = pop_head(work);
    
    int start_num = hyp.num;
    int start_pos = hyp.cell;

    //if the initial condition is not safe return
    if(!is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(start_pos), COL(start_pos), start_num))
        return 0;

    //if it is safe put the condition in the sudoku and update the masks
    cp_sudoku[start_pos] = start_num;
    update_masks(start_num, ROW(start_pos), COL(start_pos), rows_mask, cols_mask, boxes_mask);
    
    /*#pragma omp critical
    {
    printf("tid=%d\n", omp_get_thread_num());
    print_sudoku(cp_sudoku);
    sleep(1);
    }*/
    
    while(zeros){
        
        zeros = 0;

        //start the search in the cell next to the initial condition cell
        cell = start_pos +1;

        //there is a mistake: go back
        if(flag_back){
            flag_back = 0;
            
            //get next hypothesis to work on from the list 
            #pragma omp critical(hyp)
            hyp = pop_head(work);
            
            // put every element back to zero until we get to the new hypothesis cell
            for(cell = cell_aux -1; cell >= hyp.cell; cell--) {

                //if the current cell has a number greater than 0: remove that number from the sudoku and from the masks
                if(cp_sudoku[cell] > 0) {
                    
                    rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                    cp_sudoku[cell] = UNASSIGNED;

                    //when we get to the new hypothesis cell: write the new number on the sudoku, update masks, continue the search from the next cell on
                    if(cell == hyp.cell){

                        update_masks(hyp.num, ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                        cp_sudoku[cell] = hyp.num;
                        
                      /*  if(++cont == 500000){
                            #pragma omp critical
                            {
                                cont = 0;
                                system("clear");
                                print_sudoku(cp_sudoku);
                                //usleep(100000);
                            }
                        } */
                        
                        cell = hyp.cell + 1;
                        break;
                    }
                }
            }
        }
        
        //iterate the sudoku trying a number in each cell
        for(; cell < v_size; cell++){
            
            if(!cp_sudoku[cell]){ //if the cell has a zero skip it, else try a number
                
                row = ROW(cell);
                col = COL(cell);
                
                zeros++;
                first_valid = 1;
                
                //for each cell find the valid numbers, try the first valid, save the others on a work list
                for(val = 1; val <= m_size; val++){
                    if(is_safe_num(rows_mask, cols_mask, boxes_mask, row, col, val)){
                        
                        //explore only the first valid, save its value to insert at the end of the cycle
                        if(first_valid){
                            val_to_insert = val;
                            first_valid = 0;
                        }else{
                            //add the remaining valid ones to work list
                            #pragma omp critical(hyp)
                            {
                                hyp.cell = cell;
                                hyp.num = val;
                                insert_head(work, hyp);
                            }
                        }
                    }
                    //After testing all the numbers
                    //if at least one valid number was found: write it to the sudoku, update the masks, go to the next cell
                    if(val == m_size && first_valid == 0){
                        update_masks(val_to_insert, row, col, rows_mask, cols_mask, boxes_mask);
                        cp_sudoku[cell] = val_to_insert;
                        
                      /*  if(++cont == 500000){
                            #pragma omp critical
                            {   
                                cont = 0;
                                system("clear");
                                print_sudoku(cp_sudoku);
                                //usleep(100000);
                            }
                        } */
                        
                        break;
                    //if no valid number was found there must be a mistake somewhere
                    }else if(val == m_size){
                        
                        //if there is no more work to do, it's impossible to solve the sudoku from the initial condition: delete everything and return
                        if(work->head == NULL){
                            for(; cell >= start_pos; cell--){
                                if(cp_sudoku[cell] > 0){
                                    rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                                    cp_sudoku[cell] = UNASSIGNED;
                                }
                            }
                            return 0;
                        }
                        //if there's still work to do search activate: flag_back, break
                        else{
                            flag_back = 1;
                            cell_aux = cell;
                            cell = v_size; //break
                        }
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
        hyp = pop_tail(list_array[receive_from_th]);
    
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
