#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <mpi.h>
#include "list.h"

#define UNASSIGNED 0
#define UNCHANGEABLE -1

#define POS 0
#define VAL 1

#define TAG_HYP      1
#define TAG_EXIT     2
#define TAG_ASK_JOB  3
#define TAG_CP_SUD   4
#define TAG_HAVE_JOB 5

#define ROW(i) i/m_size
#define COL(i) i%m_size
#define BOX(row, col) r_size*(row/r_size)+col/r_size

#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1,p,n)-1)

void update_masks(int num, int row, int col, uint64_t *rows_mask, uint64_t *cols_mask, uint64_t *boxes_mask);
int is_safe_num( uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int solve_from(int* sudoku, int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work, int last_pos);
void init_masks(int* sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int exists_in( int index, uint64_t* mask, int num);
int* read_matrix(char *argv[]);
void print_sudoku(int *sudoku);
int int_to_mask(int num);
int new_mask( int size);
int solve(int *sudoku);

void send_ring(void * msg, int tag, int dest);
void send_Iring(void * msg, int tag, int dest, MPI_Request *request);
void delete_from(int *sudoku, int*cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int cell);
int r_size, m_size, v_size;
int id, p;
char exit_flag = 0;
int nr_it = 0, nb_sends = 0; //a eliminar

int main(int argc, char *argv[]){
    int* sudoku;

    if(argc == 2){
        sudoku = read_matrix(argv);
        //MPI initialization
        MPI_Init (&argc, &argv);
        MPI_Comm_rank (MPI_COMM_WORLD, &id);
        MPI_Comm_size (MPI_COMM_WORLD, &p);

        if(solve(sudoku))
            print_sudoku(sudoku);
        else
            printf("[%d] No solution\n", id);
        
        printf("[%d] nr_it=%d\n", id,nr_it);

        MPI_Barrier(MPI_COMM_WORLD);
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
    
    for(i = 1 + BLOCK_LOW(id, p, m_size); i < 2 + BLOCK_HIGH(id, p, m_size); i++){
        hyp.num = i;
        insert_head(work, hyp);
    }
    
    solved = solve_from(sudoku, cp_sudoku, r_mask_array, c_mask_array, b_mask_array, work, last_pos);

    if(solved)
        for(i = 0; i < v_size; i++)
            if(cp_sudoku[i] != UNCHANGEABLE)
                sudoku[i] = cp_sudoku[i];
    
    free(work);
    free(r_mask_array);
    free(c_mask_array);
    free(b_mask_array);
    free(cp_sudoku);
    
    if(solved == 1)
        return 1;
    return 0;
}

int solve_from(int* sudoku, int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work, int last_pos){
    int cell, val, flag = 1, *recv, number_amount, f_break = 0;
    MPI_Request request, req_send;
    MPI_Status status, status_send;
    int *recv_buf, *send_msg;	
    Item hyp, hyp_recv;

    hyp = pop_head(work);
    int start_pos = hyp.cell;
    
    int id_0, id_1;

    while(work->head != NULL){
        hyp = pop_head(work);
        int start_pos = hyp.cell;

        if(!is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(hyp.cell), COL(hyp.cell), hyp.num))
            continue;

        while(1){
            update_masks(hyp.num, ROW(hyp.cell), COL(hyp.cell), rows_mask, cols_mask, boxes_mask);
            cp_sudoku[hyp.cell] = hyp.num;
            
            nr_it ++;
            if(nr_it ==100000){
                printf("[%d] Running... POS:%d, VAL:%d\n", id, hyp.cell, hyp.num);
                nr_it = 0;
            }
            for(cell = hyp.cell + 1; cell < v_size; cell++){

                if(!cp_sudoku[cell]){
                    for(val = m_size; val >= 1; val--){
                        
                        if(is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(cell), COL(cell), val)){
                                if(cell == last_pos){
                                cp_sudoku[cell] = val;
                                //termination
            

                                send_ring(&id, TAG_EXIT, -1);
                                return 1;
                                }
                            
                            hyp.cell = cell;
                            hyp.num = val;
                            insert_head(work, hyp);			
                        
                            flag = 0;
                            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);	
                            
                            if(flag && status.MPI_TAG != -1){
                                
                                MPI_Get_count(&status, MPI_INT, &number_amount);
                                recv = (int *) malloc(number_amount*sizeof(int));
                                //printf("here%d\n",number_amount);
                                MPI_Recv(recv, number_amount, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                                flag = 0;
                            
                                //printf("[%d] received msg with tag %d from %d originated in %d with size %d\n", id,status.MPI_TAG, status.MPI_SOURCE, recv[1], number_amount);
                                switch(status.MPI_TAG){
                                    case TAG_EXIT:
                                        send_ring(&id, TAG_EXIT, -1);
                                        return 0;
                                        break;
                                
                                    case TAG_ASK_JOB:
                                        //printf("[%d] proc %d asked for a job\n", id, recv[1]);
                                        if(id != recv[1]){
                                            if(work->tail != NULL){
                                                //printf("[%d] I have work to %d\n", id, recv[1]);
                                                send_ring(&id, TAG_HAVE_JOB, recv[1]);
                                            }else{
                                                //printf("[%d] I dont have work to %d, sending forward",id, recv[1]);
                                                send_ring(&recv[0], TAG_ASK_JOB, recv[1]);
                                            }
                                        }else
                                            return 0;
                                        break;
                                    case TAG_HAVE_JOB:
                                        if(id == recv[1]){
                                            //printf("[%d] received control\n", id);
                                            MPI_Send(recv, 2, MPI_INT, recv[0], TAG_HYP, MPI_COMM_WORLD);
                                        }else{
                                            //printf("[%d] forwarding control from %d\n", id, recv[1]);
                                            send_ring(&recv[0], TAG_HAVE_JOB, recv[1]);
                                        }
                                        break;
                                        
                                    case TAG_HYP:
                                        if(id != recv[1]){
                                            //printf("[%d] received initiation to %d\n", id, recv[1]);
                                            send_msg = (int*)malloc((v_size+2)*sizeof(int));
                                            Item hyp_send = pop_tail(work);
                                            memcpy(send_msg, &hyp_send, sizeof(Item));
                                            memcpy((send_msg+2), cp_sudoku, v_size*sizeof(int));
                                            MPI_Send(send_msg, (v_size+2), MPI_INT, status.MPI_SOURCE, TAG_HYP, MPI_COMM_WORLD);
                                            //printf("[%d] transfer to %d done\n", id, recv[1]);
                                        }else{
                                            memcpy(&hyp_recv, recv, sizeof(Item));
                                            memcpy(cp_sudoku, (recv+2), v_size*sizeof(int));
                                            delete_from(sudoku, cp_sudoku, rows_mask, cols_mask, boxes_mask, hyp_recv.cell);
                                            insert_head(work, hyp_recv);
                                            //printf("[%d] - DONE receiving\n" ,id);
                                        }
                                        break;
                                        
                                    default:
                                        //printf("[%d] undefined behaviour from %d\n", id, recv[1]);
                                        break;
                                }

                                
                            }
                        }
                    }                
                        
                    if(work->head == NULL){
                        for(cell = v_size - 1; cell >= start_pos; cell--)
                            if(cp_sudoku[cell] > 0){
                                rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                                cp_sudoku[cell] = UNASSIGNED;
                            }
                        f_break = 1;
                        break;
                    }else
                        break;
                }
            }
            
            if(f_break){
                f_break = 0;
                break;
            }
            
            hyp = pop_head(work);
            
            for(cell--; cell >= hyp.cell; cell--){
                if(cp_sudoku[cell] > 0) {
                    rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                    cp_sudoku[cell] = UNASSIGNED;
                }
            }
        }
    }
}

int exists_in(int index, uint64_t* mask, int num) {
    int res, masked_num = int_to_mask(num);

    res = mask[index] | masked_num;
    if(res != mask[index]) 
        return 0;
    return 1;
}

int is_safe_num(uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num) {
    return !exists_in(row, rows_mask, num) && !exists_in(col, cols_mask, num) && !exists_in(BOX(row, col), boxes_mask, num);
}

int new_mask(int size) {
    return (0 << (size-1));
}

int int_to_mask(int num) {
    return (1 << (num-1));
}

void init_masks(int* sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
    int i, row, col;

    for(i = 0; i < m_size; i++){
        rows_mask[i]  = UNASSIGNED;
        cols_mask[i]  = UNASSIGNED;
        boxes_mask[i] = UNASSIGNED;
    }

    for(i = 0; i < v_size; i++){
        if(sudoku[i]){
            row = ROW(i);
            col = COL(i);
            
            update_masks(sudoku[i], row, col, rows_mask, cols_mask, boxes_mask);
        }
    }
}

void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
    int num_mask = int_to_mask(num);
    rows_mask[row] ^= num_mask;
    cols_mask[col] ^= num_mask;
    boxes_mask[BOX(row, col)] ^= num_mask;
}

void update_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
    int new_mask = int_to_mask(num);
    rows_mask[row] |= new_mask;
    cols_mask[col] |= new_mask;
    boxes_mask[BOX(row, col)] |= new_mask;
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

void delete_from(int* sudoku, int *cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int cell){
    int i;

    init_masks(sudoku, rows_mask, cols_mask, boxes_mask);

    i = v_size;
    while(i >= cell){
        if(cp_sudoku[i] > 0)
        cp_sudoku[i] = UNASSIGNED;
        i--;
    }

    for(i = 0; i < cell; i++)
        if(cp_sudoku[i] > 0)
            update_masks(cp_sudoku[i], ROW(i), COL(i), rows_mask, cols_mask, boxes_mask);
}

void send_ring(void *msg, int tag, int dest ){
    int msg_send[2];
    msg_send[0] =*((int*) msg);
    msg_send[1] = dest;
    
    if(id == p-1)
        MPI_Send(msg_send, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    else
        MPI_Send(msg_send, 2, MPI_INT, id+1, tag, MPI_COMM_WORLD);
}


void send_Iring(void *msg, int tag, int dest , MPI_Request *request){
    int msg_send[2];
    msg_send[0] =*((int*) msg);
    msg_send[1] = dest;
    
    if(id == p-1)
        MPI_Isend(msg_send, 2, MPI_INT, 0, tag, MPI_COMM_WORLD, request);
    else
        MPI_Isend(msg_send, 2, MPI_INT, id+1, tag, MPI_COMM_WORLD, request);
} 
