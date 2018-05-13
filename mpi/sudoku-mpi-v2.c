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

#define ROW(i) i/m_size
#define COL(i) i%m_size
#define BOX(row, col) r_size*(row/r_size)+col/r_size

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)

void update_masks(int num, int row, int col, uint64_t *rows_mask, uint64_t *cols_mask, uint64_t *boxes_mask);
int is_safe_num( uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int solve_from(int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work, int last_pos);
void delete_from(int *cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int cell);
void init_masks(int* sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int exists_in( int index, uint64_t* mask, int num);
int* read_matrix(char *argv[]);
void print_sudoku(int *sudoku);
int int_to_mask(int num);
int new_mask( int size);
int solve(int *sudoku);

int r_size, m_size, v_size;
int id, p;

int nr_it = 0; //a eliminar

int main(int argc, char *argv[]){
    int result, *sudoku;

    if(argc == 2){

        sudoku = read_matrix(argv);

        MPI_Init (&argc, &argv);
        MPI_Comm_rank (MPI_COMM_WORLD, &id);
        MPI_Comm_size (MPI_COMM_WORLD, &p);
        
        result = solve(sudoku);

        printf("process %d => nr_it=%d\n", id, nr_it);
	MPI_Barrier(MPI_COMM_WORLD);

        if(result)
      	    print_sudoku(sudoku);

        fflush(stdout);
        MPI_Finalize();

    }else
        printf("invalid input arguments.\n");

    free(sudoku);

    return 0;
}

int solve(int* sudoku){
    int i, flag_start = 0, solved = 0, start_pos, start_num, last_pos, pos_aux, val_aux, flag_enter = 1;
    int low_value, high_value, result, flag, recv, recv_hyp[2];

    MPI_Request request_send, request_recv_exit, request_recv_hyp, request_recv_cp;
    MPI_Status status;
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
                start_pos = i;
            }
            last_pos = i;
        }
    }

    init_masks(sudoku, r_mask_array, c_mask_array, b_mask_array);

    low_value = 1 + BLOCK_LOW(id,p,m_size);
    high_value = 2 + BLOCK_HIGH(id,p,m_size);
    
    start_num = low_value;
    val_aux = start_num;
    pos_aux = start_pos;

    MPI_Irecv(recv_hyp, 2, MPI_INT, MPI_ANY_SOURCE, TAG_HYP, MPI_COMM_WORLD, &request_recv_hyp);
    MPI_Irecv(cp_sudoku, v_size, MPI_INT, MPI_ANY_SOURCE, TAG_CP_SUD, MPI_COMM_WORLD, &request_recv_cp);

    flag = 0;
    while(!solved){

	if(flag_enter){
	    hyp.cell = start_pos;
            hyp.num = start_num;
            insert_head(work, hyp);

	    flag_enter = 0;

<<<<<<< HEAD:mpi/sudoku-mpi-2nd-approach.c
        if((result = solve_from(cp_sudoku, r_mask_array, c_mask_array, b_mask_array, work, last_pos))) {
            solved = 1;
            for(i = 0; i < v_size; i++)
                if(cp_sudoku[i] != UNCHANGEABLE)
                    sudoku[i] = cp_sudoku[i];
                
                for(i = id + 1; i != id; i++){
                    if(i == id)
                        break;
                    if(i >= p)
                        i = 0;
                    
                    MPI_Send(&i, 1, MPI_INT, i, id, MPI_COMM_WORLD);
                }
                
        }else{
            if(result == -1)
=======
	    printf("[%d] cell = %d, val = %d\n", id, start_pos, start_num);

            if((result = solve_from(cp_sudoku, r_mask_array, c_mask_array, b_mask_array, work, last_pos)) == 1) {
	    	solved = 1;
            	for(i = 0; i < v_size; i++)
                    if(cp_sudoku[i] != UNCHANGEABLE)
                    	sudoku[i] = cp_sudoku[i];

                for(i = 0; i < p; i++)
		    if(i != id)
                        MPI_Isend(&i, 1, MPI_INT, i, TAG_EXIT, MPI_COMM_WORLD, &request_send);
            }
	}

	if(result != 1){
	    MPI_Test(&request_recv_exit, &flag, &status);
	    if(flag){
		flag = 0;
		MPI_Irecv(&recv, 1, MPI_INT, MPI_ANY_SOURCE, TAG_EXIT, MPI_COMM_WORLD, &request_recv_exit);
		if(status.MPI_TAG == TAG_EXIT)
		    return 0;
	    }
	    
	    if(result == -1)
>>>>>>> b1bf1b4e32e14deb82a9a9b4a09387234b7ff718:mpi/sudoku-mpi-v2.c
                return 0;

	    start_num = val_aux;
	    if(start_num < high_value){
		start_num++;
            	val_aux = start_num;
	    }
  
	    if(start_num == high_value){
            	for(i = 0; i < p; i++)
                    if(i != id)
                    	MPI_Send(&i, 1, MPI_INT, i, TAG_ASK_JOB, MPI_COMM_WORLD);
	    }else
		flag_enter = 1;

	    MPI_Test(&request_recv_hyp, &flag, &status);
            if(flag){
		flag = 0;
		MPI_Irecv(recv_hyp, 2, MPI_INT, MPI_ANY_SOURCE, TAG_HYP, MPI_COMM_WORLD, &request_recv_hyp);
                if(status.MPI_TAG == TAG_HYP){
                    printf("[%d]recv hyp from [%d] cell = %d, val = %d\n", id, status.MPI_SOURCE, recv_hyp[POS], recv_hyp[VAL]);
                    
		    start_pos = recv_hyp[POS];
                    start_num = recv_hyp[VAL];
                }
            }

	    MPI_Test(&request_recv_cp, &flag, &status);
            if(flag){
		flag = 0;
		MPI_Irecv(cp_sudoku, v_size, MPI_INT, MPI_ANY_SOURCE, TAG_CP_SUD, MPI_COMM_WORLD, &request_recv_cp);
		flag_enter = 1;
                if(status.MPI_TAG == TAG_CP_SUD){
		    delete_from(cp_sudoku, r_mask_array, c_mask_array, b_mask_array, recv_hyp[POS]);
		    print_sudoku(cp_sudoku);
                }
            }

	    //start_num = high_value - 1;
	    //start_pos = pos_aux;

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
    int cell, val, recv, flag, result, response[2];
    MPI_Request request;
    MPI_Status status;
    Item hyp;
    
    hyp = pop_head(work);
    int start_pos = hyp.cell;

    if(!is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(hyp.cell), COL(hyp.cell), hyp.num))
        return 0;

    flag = -1;
    while(1){
	if(flag){
	    MPI_Irecv(&recv, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
	    flag = 0;
	}
        MPI_Test(&request, &flag, &status);
<<<<<<< HEAD:mpi/sudoku-mpi-2nd-approach.c
        if(flag){
            printf("rank = %d => renv: %d\n", id, recv);
            return -1;
        }
        
=======
	if(flag){
	    if(status.MPI_TAG == TAG_EXIT){
		printf("[%d] %d asked for exit\n", id, status.MPI_SOURCE);
	    	return -1;
	    }else if(status.MPI_TAG == TAG_ASK_JOB){
		printf("[%d] %d asked for a job\n", id, status.MPI_SOURCE);
		if(work->head != NULL){
		    Item hyp_send = pop_head(work);
		    response[POS] = hyp_send.cell;
		    response[VAL] = hyp_send.num;

		    MPI_Isend(response, 2, MPI_INT, status.MPI_SOURCE, TAG_HYP, MPI_COMM_WORLD, &request);
                    MPI_Isend(cp_sudoku, v_size, MPI_INT, status.MPI_SOURCE, TAG_CP_SUD, MPI_COMM_WORLD, &request);
		}
	    }
	}

>>>>>>> b1bf1b4e32e14deb82a9a9b4a09387234b7ff718:mpi/sudoku-mpi-v2.c
        update_masks(hyp.num, ROW(hyp.cell), COL(hyp.cell), rows_mask, cols_mask, boxes_mask);
        cp_sudoku[hyp.cell] = hyp.num;
        
        nr_it ++;
        
        for(cell = hyp.cell + 1; cell < v_size; cell++){

            if(!cp_sudoku[cell]){
                for(val = m_size; val >= 1; val--){
                    
                    if(is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(cell), COL(cell), val)){
                         if(cell == last_pos){
                            cp_sudoku[cell] = val;
                            return 1;
                         }
                        
                        hyp.cell = cell;
                        hyp.num = val;
                        insert_head(work, hyp);
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

void delete_from(int *cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int cell){
    int i;
    
    i = v_size;
    while(i >= cell){
        if(cp_sudoku[i] > 0)
            cp_sudoku[i] = UNASSIGNED;
    	i--;
    }

    for(i = 0; i < cell; i++)
	update_masks(cp_sudoku[i], ROW(i), COL(i), rows_mask, cols_mask, boxes_mask);
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