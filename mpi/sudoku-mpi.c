#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <mpi.h>
#include <omp.h>

#include "list.h"

#define UNASSIGNED 0
#define UNCHANGEABLE -1

#define POS 0
#define VAL 1

#define TAG_HYP     1
#define TAG_EXIT    2
#define TAG_ASK_JOB 3

#define ROW(i) i/m_size
#define COL(i) i%m_size
#define BOX(row, col) r_size*(row/r_size)+col/r_size

#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1,p,n)-1)

int solve_from(int* sudoku, int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work, int last_pos);
void delete_from(int* sudoku, int *cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int cell);
void update_masks(int num, int row, int col, uint64_t *rows_mask, uint64_t *cols_mask, uint64_t *boxes_mask);
void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int is_safe_num( uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num);
void init_masks(int* sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int exists_in( int index, uint64_t* mask, int num);
void send_ring(void *msg, int tag, int dest);
int* read_matrix(char *argv[]);
void print_sudoku(int *sudoku);
int int_to_mask(int num);
int new_mask( int size);
int solve(int *sudoku);
Item invalid_hyp(void);

int r_size, m_size, v_size, id, p;
int nr_it = 0;

int main(int argc, char *argv[]){
    int* sudoku, result, total;

    if(argc == 2){
        sudoku = read_matrix(argv);
        
        MPI_Init (&argc, &argv);
        MPI_Comm_rank (MPI_COMM_WORLD, &id);
        MPI_Comm_size (MPI_COMM_WORLD, &p);

        result = solve(sudoku);
        
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Allreduce(&result, &total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        
        if(!total && !id)
            printf("No solution\n");
        else if(total && result)
            print_sudoku(sudoku);
        
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Allreduce(&nr_it, &total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        
        if(!id)
            printf("nr_it=%d\n", total);

        fflush(stdout);
        MPI_Finalize();
        
    }else
        printf("invalid input arguments.\n");

    free(sudoku);    

    return 0;
}

int solve(int* sudoku){
    int i, last_pos, flag_start = 0, solved = 0;
    Item hyp;
    
    //init work lists, one for each process.
    //A work list contains the pairs (cell_id, number) from wich serial DFS search must still be performed
    //(in other words it cointains the root values of the unexplored parts of the search tree)
    uint64_t *r_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    uint64_t *c_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    uint64_t *b_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    int *cp_sudoku = (int*) malloc(v_size * sizeof(int));
    List *work = init_list();

    for(i = 0; i < v_size; i++) {
        if(sudoku[i])
            cp_sudoku[i] = UNCHANGEABLE;
        else{
            cp_sudoku[i] = UNASSIGNED;
            if(!flag_start){
                flag_start = 1;
                hyp.cell = i;
            }
            last_pos = i;
        }
    }

    init_masks(sudoku, r_mask_array, c_mask_array, b_mask_array);
    
    //calculate the low and high values for the first cell for each precess
    //and insert it in the work list
    for(i = 1 + BLOCK_HIGH(id, p, m_size); i >= 1 + BLOCK_LOW(id, p, m_size); i--){
        hyp.num = i;
        insert_head(work, hyp);
    }

    // try to solve sudoku
    solved = solve_from(sudoku, cp_sudoku, r_mask_array, c_mask_array, b_mask_array, work, last_pos);

    if(solved){
        //if the solution is found copy the solution to be retrieved
        #pragma omp parallel for
        for(i = 0; i < v_size; i++)
            if(cp_sudoku[i] != UNCHANGEABLE)
                sudoku[i] = cp_sudoku[i];
    }
    
    free(work);
    free(r_mask_array);
    free(c_mask_array);
    free(b_mask_array);
    free(cp_sudoku);

    return solved;
}

int solve_from(int* sudoku, int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work, int last_pos){
    int i, cell, val, number_amount, flag = 0, no_sol_count, len;
    
    MPI_Request request;
    MPI_Status status;
    Item hyp, no_hyp = invalid_hyp();
    
    while(1){
        while(work->head != NULL){
            hyp = pop_head(work);
            len = work->len;
            int start_pos = hyp.cell;

            if(!is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(hyp.cell), COL(hyp.cell), hyp.num))
                continue;

            while(1){
                nr_it++;
                MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
                if(flag && status.MPI_TAG != -1){
                    flag = 0;
                    MPI_Get_count(&status, MPI_INT, &number_amount);
                    int* number_buf = (int*)malloc(number_amount * sizeof(int));
                    MPI_Recv(number_buf, number_amount, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    
                    if(status.MPI_TAG == TAG_EXIT){
                        len = work->len;
                        printf("size=%d\n", len);
                        send_ring(&id, TAG_EXIT, -1);
                        return 0;
                    }else if(status.MPI_TAG == TAG_ASK_JOB){
                        if(work->tail != NULL){
                            Item hyp_send = pop_tail(work);
                            int* send_msg = (int*)malloc((v_size+2)*sizeof(int));
                            memcpy(send_msg, &hyp_send, sizeof(Item));
                            memcpy((send_msg+2), cp_sudoku, v_size*sizeof(int));
                            MPI_Send(send_msg, (v_size+2), MPI_INT, status.MPI_SOURCE, TAG_HYP, MPI_COMM_WORLD);
                            free(send_msg);
                        }else
                            MPI_Send(&no_hyp, 2, MPI_INT, status.MPI_SOURCE, TAG_HYP, MPI_COMM_WORLD);
                    }
                    free(number_buf);
                }
            
                update_masks(hyp.num, ROW(hyp.cell), COL(hyp.cell), rows_mask, cols_mask, boxes_mask);
                cp_sudoku[hyp.cell] = hyp.num;
                
                for(cell = hyp.cell + 1; cell < v_size; cell++){
                    if(cp_sudoku[cell])
                        continue;
                    
                    for(val = m_size; val >= 1; val--){
                        if(is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(cell), COL(cell), val)){
                            if(cell == last_pos){
                                cp_sudoku[cell] = val;
                                send_ring(&id, TAG_EXIT, -1);
                                return 1;
                            }
                            
                            hyp.cell = cell;
                            hyp.num = val;
                            insert_head(work, hyp);
                        }
                            
                    }
                    break;
                }
                
                /*if(work->len == len){
                    for(cell = v_size - 1; cell >= start_pos; cell--)
                        if(cp_sudoku[cell] > 0){
                            rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                            cp_sudoku[cell] = UNASSIGNED;
                        }
                    break;
                }*/
                
                if(work->head == NULL)
                    break;
                
                hyp = pop_head(work);
                
                for(cell--; cell >= hyp.cell; cell--){
                    if(cp_sudoku[cell] > 0) {
                        rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                        cp_sudoku[cell] = UNASSIGNED;
                    }   
                }
            }
        }

        no_sol_count = 0;
        
        if(p == 1)
            return 0;

        for(i = id+1;; i++){
            if(i == p) i = 0;
            if(i == id) continue;

            MPI_Send(&i, 1, MPI_INT, i, TAG_ASK_JOB, MPI_COMM_WORLD);

            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &number_amount);
            int* number_buf = (int*)malloc(number_amount * sizeof(int));
            MPI_Recv(number_buf, number_amount, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            if(status.MPI_TAG == TAG_HYP && number_amount != 2){
                Item hyp_recv;
                memcpy(&hyp_recv, number_buf, sizeof(Item));
                memcpy(cp_sudoku, (number_buf+2), v_size*sizeof(int));
                
                delete_from(sudoku, cp_sudoku, rows_mask, cols_mask, boxes_mask, hyp_recv.cell);
                
                insert_head(work, hyp_recv);
                free(number_buf);
                break;
            }else if(status.MPI_TAG == TAG_HYP && number_amount == 2){
                no_sol_count++;
            }else if(status.MPI_TAG == TAG_EXIT){
                printf("size=%d\n", work->len);
                send_ring(&id, TAG_EXIT, -1);
                return 0;
            }else if(status.MPI_TAG == TAG_ASK_JOB){
                MPI_Send(&no_hyp, 2, MPI_INT, status.MPI_SOURCE, TAG_HYP, MPI_COMM_WORLD);
            }
            
            if(no_sol_count == p-1 && id == 0){
                printf("size=%d\n", work->len);
                send_ring(&id, TAG_EXIT, -1);
                return 0;
            }

            free(number_buf);
        }
    }
}

//when a process receives a new hypothesis and the corresponding cp_sudoku, then we has to clear all the work
//that the sender process have done after the hypothesis's position
void delete_from(int* sudoku, int *cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int cell){
    int i;
    
    //initialize the masks from the sudoku read from the file
    init_masks(sudoku, rows_mask, cols_mask, boxes_mask);
    
    //clear all work done by the sender process
    #pragma omp parallel for
    for(i = v_size; i >= cell; i--)
        if(cp_sudoku[i] > 0)
            cp_sudoku[i] = UNASSIGNED;

    //initialize the masks from the cp_sudoku received
    for(i = 0; i < cell; i++)
        if(cp_sudoku[i] > 0)
            update_masks(cp_sudoku[i], ROW(i), COL(i), rows_mask, cols_mask, boxes_mask);
}

//create an invalid hypothesis
Item invalid_hyp(void){
    Item item;
    item.cell = -1;
    item.num = -1;
    return item;
}

void send_ring(void *msg, int tag, int dest){
    int msg_send[2];
    msg_send[0] =*((int*) msg);
    msg_send[1] = dest;
    
    if(id == p-1)
        MPI_Send(msg_send, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    else
        MPI_Send(msg_send, 2, MPI_INT, id+1, tag, MPI_COMM_WORLD);
}

//test if a number (num) exists in a row, column or box. To check if it exists in a row, the mask of that row is passed as input
int exists_in(int index, uint64_t* mask, int num) {
    int res, masked_num = int_to_mask(num);

    res = mask[index] | masked_num;  //example row_mask = 1010 ; number to check = 0100 (3) then row_masl | num = 1110
    if(res != mask[index])           // since the result is different from the initial mask, it is safe to put 3 in this row/col/box
        return 0;
    return 1;
}

//check if a given number is safe in a given cell (row,col) for that check safety in corresponding row, column and box
int is_safe_num(uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num) {
    return !exists_in(row, rows_mask, num) && !exists_in(col, cols_mask, num) && !exists_in(BOX(row, col), boxes_mask, num);
}

//initialize a mask
int new_mask(int size) {
    return (0 << (size-1));
}

//convert an integer to mask ex: 3 = 0100
int int_to_mask(int num) {
    return (1 << (num-1));
}

//initialize the masks from the sudoku read from the file
void init_masks(int* sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
    int i;

    #pragma omp parallel for
    for(i = 0; i < m_size; i++){
        rows_mask[i]  = UNASSIGNED;
        cols_mask[i]  = UNASSIGNED;
        boxes_mask[i] = UNASSIGNED;
    }

    for(i = 0; i < v_size; i++)
        if(sudoku[i])
            update_masks(sudoku[i], ROW(i), COL(i), rows_mask, cols_mask, boxes_mask);
}

//remove number from the masks
void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
    int num_mask = int_to_mask(num);
    rows_mask[row] ^= num_mask;
    cols_mask[col] ^= num_mask;
    boxes_mask[BOX(row, col)] ^= num_mask;
}

//add new number to the masks
void update_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
    int new_mask = int_to_mask(num); //convert number found to mask ex: if dim=4x4, 3 = 0010
    rows_mask[row] |= new_mask;      //to add the new number to the current row's mask use bitwise OR
    cols_mask[col] |= new_mask;      //ex row_mask = 0101 ; number to add: 0010 --> row_mask OR num = 0111
    boxes_mask[BOX(row, col)] |= new_mask;
}

//read the input file
int* read_matrix(char *argv[]) {
    FILE *fp;
    size_t characters, len = 1;
    char *line = NULL, aux[3];
    int i, j, k, l;

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
