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


#define TAG_HYP     1
#define TAG_EXIT    2
#define TAG_ASK_JOB 3
#define TAG_CP_SUD  4
#define TAG_HAVE_JOB 5


#define ROW(i) i/m_size
#define COL(i) i%m_size
#define BOX(row, col) r_size*(row/r_size)+col/r_size

#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1,p,n)-1)



void update_masks(int num, int row, int col, uint64_t *rows_mask, uint64_t *cols_mask, uint64_t *boxes_mask);
int is_safe_num( uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int solve_from(int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work, int last_pos);
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
    int i, flag_start = 0, solved = 0, start_pos, start_num, last_pos;
    Item hyp;
    int result, flag = 1;
    uint64_t *r_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
	int low_value, high_value;
    uint64_t *c_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    uint64_t *b_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    int      *cp_sudoku    = (int*)      malloc(v_size * sizeof(int));
    MPI_Request request_recv, request_send;
	MPI_Status status_recv, status_send;
	int flag_recv = 0, flag_send = 0, flag_done = 0;
	int recv[2], number_amount, msg[2];
	List *work = init_list();
    
    for(i = 0; i < v_size; i++) {
        if(sudoku[i])
            cp_sudoku[i] = UNCHANGEABLE;
        else{
            cp_sudoku[i] = UNASSIGNED;
            if(!flag_start){
                flag_start = 1;
                start_pos = i;
            }
            last_pos = i;
        }
    }
    
    init_masks(sudoku, r_mask_array, c_mask_array, b_mask_array);

	low_value = 1 + BLOCK_LOW(id, p, m_size);
    high_value = 2 + BLOCK_HIGH(id, p, m_size);
    start_num = low_value;

    while(1) {

        if(!solved){
            hyp.cell = start_pos;
            hyp.num = start_num;

            insert_head(work, hyp);
        
            if((result = solve_from(cp_sudoku, r_mask_array, c_mask_array, b_mask_array, work, last_pos)) == 1) {
                solved = 1;
                for(i = 0; i < v_size; i++)
                    if(cp_sudoku[i] != UNCHANGEABLE)
                        sudoku[i] = cp_sudoku[i];
            
			}

		}

			start_num++;
		printf("[%d] %d, high: %d\n",id, start_num, high_value);		

		if(start_num >=high_value-1 || result != 1){
			flag_send = 0;
			flag_recv = 1;
			flag_done = 0;
			printf("[%d] DONE\n", id);
			if(result == 0){
			  

				printf("[%d] have to find job\n", id);
	   		  //MPI_Recv(&recv, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status_recv);
			  while(!flag_done){
				
				if(flag_recv){
					MPI_Irecv(&recv, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request_recv);
					flag_recv = 0;
				}
				if(!flag_recv && !flag_send){
					
			  		send_ring(&id, TAG_ASK_JOB, id);
					
			  		printf("[%d] waiting for job\n", id);
					flag_send = 1;
				}
				
				
				MPI_Test(&request_recv, &flag_recv, &status_recv);
				/*if(flag_send){
					send_Iring(&id, TAG_ASK_JOB, id, &request_send);
					printf("[%d] waiting for job\n", id);
					flag_send = 0;
				}
				if(flag_recv){
					MPI_Irecv(&recv, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request_recv);
					flag_recv = 0;
				}
				MPI_Test(&request_recv, &flag_recv, &status_recv);
				*/if(flag_recv){
					if(status_recv.MPI_TAG == TAG_HAVE_JOB){
						printf("[%d] received control msg job from %d \n", id,recv[0]);

						MPI_Send(recv, 2, MPI_INT, recv[0], TAG_HYP, MPI_COMM_WORLD);


						MPI_Probe(recv[0], TAG_HYP, MPI_COMM_WORLD, &status_recv);



               		 	MPI_Get_count(&status_recv, MPI_INT, &number_amount);
          		      	int* number_buf = (int*)malloc(number_amount * sizeof(int));


						MPI_Recv(number_buf, number_amount, MPI_INT, recv[0], TAG_HYP, MPI_COMM_WORLD, &status_recv);

						Item hyp_recv;
                		memcpy(&hyp_recv, number_buf, sizeof(Item));
            	    	memcpy(cp_sudoku, (number_buf+2), v_size*sizeof(int));
				
						delete_from(sudoku, cp_sudoku, r_mask_array, c_mask_array, b_mask_array, hyp_recv.cell);
						insert_head(work, hyp_recv);
						printf("[%d] - DONE receiving\n" ,id);
						free(number_buf);

						flag_done = 1;
					}else if(status_recv.MPI_TAG == TAG_EXIT){

						printf("1- [%d] received exit from %d\n", id, recv[0]);
						send_ring(&id, TAG_EXIT, -1);	
						return 0;
					}
					flag_recv = 0;
				  }
				}
				}else if(result == -1){
					return 0;
				}else if(result == 1){
					return 1;
				}
			  }
			
       }

    

    free(work);
    free(r_mask_array);
    free(c_mask_array);
    free(b_mask_array);
    free(cp_sudoku);
    
    if(solved)
        return 1;
    return 0;
}

int solve_from(int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work, int last_pos) {
    int cell, val, flag = 1, recv[2];
	MPI_Request request, req_send;
	MPI_Status status, status_send;
	
    Item hyp;
    
    hyp = pop_head(work);
    int start_pos = hyp.cell;

    if(!is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(hyp.cell), COL(hyp.cell), hyp.num))
        return 0;

    while(1){
        update_masks(hyp.num, ROW(hyp.cell), COL(hyp.cell), rows_mask, cols_mask, boxes_mask);
        cp_sudoku[hyp.cell] = hyp.num;
        
        nr_it ++;
        
        for(cell = hyp.cell + 1; cell < v_size; cell++){

            if(!cp_sudoku[cell]){
                for(val = m_size; val >= 1; val--){
                    
                    if(is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(cell), COL(cell), val)){
                         if(cell == last_pos){
                            cp_sudoku[cell] = val;
							//termination
							printf("[%d] - ended\n", id);
							send_ring(&id, TAG_EXIT, -1);
							printf("[%d] - sent exit\n", id);
                            return 1;
                         }
                        
                        hyp.cell = cell;
                        hyp.num = val;
                        insert_head(work, hyp);
						if(flag){
							MPI_Irecv(&recv, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
							flag = 0;
						}
						MPI_Test(&request, &flag, &status);
						if(flag){
							if(status.MPI_TAG == TAG_EXIT){
								
								printf("[%d] received exit from %d\n", id, recv[0]);	
								send_ring(&id, TAG_EXIT, -1);
								return -1;

								
							}else if(status.MPI_TAG ==TAG_ASK_JOB){
								printf("[%d] - received ask from %d\n", id, recv[1]);
								if(work->head !=NULL){
									printf("[%d] I have work to %d\n", id,recv[1]);
									int msg[2];
									send_ring(&id, TAG_HAVE_JOB, recv[1]);
									MPI_Recv(&msg,2,MPI_INT, recv[1], TAG_HYP, MPI_COMM_WORLD, &status);
									printf("[%d] - start received", id);
									int* send_msg = (int*)malloc((v_size+2)*sizeof(int));
                     				Item hyp_send = pop_head(work);
                     				memcpy(send_msg, &hyp_send, sizeof(Item));
                     				memcpy((send_msg+2), cp_sudoku, v_size*sizeof(int));
				                     MPI_Send(send_msg, (v_size+2), MPI_INT, recv[1], TAG_HYP, MPI_COMM_WORLD);

                     				free(send_msg);
								}else{
									send_ring(&recv[0], TAG_ASK_JOB, recv[1]);
								}
							}else if(status.MPI_TAG == TAG_HAVE_JOB){
								send_ring(&recv[0], TAG_HAVE_JOB, recv[1]);
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
                    return 0;
                }else
                    break;
            }
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
	printf("%d\n", id);

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
	//  printf("SENT\n");
	if(id == p-1)
        MPI_Send(msg_send, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    else
        MPI_Send(msg_send, 2, MPI_INT, id+1, tag, MPI_COMM_WORLD);
}


void send_Iring(void *msg, int tag, int dest , MPI_Request *request){
  	int msg_send[2];
	msg_send[0] =*((int*) msg);
	msg_send[1] = dest;
	//  printf("SENT\n");
	if(id == p-1)
        MPI_Isend(msg_send, 2, MPI_INT, 0, tag, MPI_COMM_WORLD, request);
    else
        MPI_Isend(msg_send, 2, MPI_INT, id+1, tag, MPI_COMM_WORLD, request);
}
