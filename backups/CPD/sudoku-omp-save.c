#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <omp.h>
#include <time.h>

#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define ROW(i) i/m_size
#define COL(i) i%m_size
#define BOX(row, col) r_size*(row/r_size)+col/r_size

int r_size, m_size, v_size;

struct node {
    int r_mask;
    int c_mask;
    int b_mask;
    int pos, val;
    struct node *next;
};

int* read_matrix(char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int* sudoku);
int* solve_from( int start_pos, int start_num, int* sudoku, int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
int new_mask( int size);
int int_to_mask(int num);
void init_masks(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
int is_safe_num( int* rows_mask, int* cols_mask, int* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
void print_sudoku(int* sudoku);
struct node* add_node(struct node* n, int *r_mask, int *c_mask, int *b_mask, int pos, int val);
void update_elem_mask(int *r_mask, int *c_mask, int *b_mask, struct node* n);
void print_mask(int*mask);
int main(int argc, char *argv[]) {
    clock_t start, end;
    int result;
    int* sudoku;
    double time;

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

        //verify_sudoku(sudoku, r_size) == 1 ? printf("corret, ") : printf("wrong, ");
        result == 1 ? printf("solved!\n") : printf("no solution!\n");

        time = (double) (end-start)/CLOCKS_PER_SEC;
        printf("took %lf sec (%.3lf ms).\n\n", time, time*1000.0);


        free(sudoku);
    }else
        printf("invalid input arguments.\n");

    return 0;
}

int solve(int* sudoku){
    int cp_sudoku[v_size];
    int cell, pos, val = 0, fst_pos = 0, done = 1;
    int* rows_mask;
    int* cols_mask;
    int* boxes_mask;
    struct node *top_node, *task_node;
    rows_mask = (int*) malloc(m_size * sizeof(int));
    cols_mask = (int*) malloc(m_size * sizeof(int));
    boxes_mask = (int*) malloc(m_size * sizeof(int));
    // init each vector for each thread
    for(cell = 0; cell < v_size; cell++){
        if(sudoku[cell])
            cp_sudoku[cell] = UNCHANGEABLE;
        else{
            // first pos empty
            if(!fst_pos){
                fst_pos = !fst_pos;
                pos = cell;
            }
            cp_sudoku[cell] = UNASSIGNED;
        }
    }

    #pragma omp parallel firstprivate(top_node, pos, val, done, rows_mask, cols_mask, boxes_mask) shared(task_node)
    {
      init_masks(sudoku, rows_mask, cols_mask, boxes_mask);
    top_node = add_node(NULL, rows_mask, cols_mask, boxes_mask, pos, val);
    task_node = add_node(NULL, rows_mask, cols_mask, boxes_mask, pos, val);
    #pragma omp single nowait
    {

    while(top_node->pos < v_size){
        done = 1;

         if(top_node->val < m_size){
              val = ++top_node->val;
              pos = top_node->pos;

             if(is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(pos), COL(pos), val)){
               #pragma omp task
               //printf("tid: %d, pos: %d, val: %d\n", omp_get_thread_num(), ++top_node->pos, ++top_node->val);
               #pragma omp taskwait
               #pragma omp task shared(task_node) firstprivate(rows_mask, cols_mask, boxes_mask, pos, val, top_node  )
                {
                //print_mask(rows_mask);
                  #pragma omp critical
                  {
                    task_node = add_node(task_node, rows_mask, cols_mask, boxes_mask, pos, val);
                    update_masks(val, ROW(pos), COL(pos), rows_mask, cols_mask, boxes_mask);
                    //sudoku[task_node->pos] =  task_node->val;
                  }
                }
                #pragma omp taskwait

                    while(cp_sudoku[++pos] == -1);
                    top_node = add_node(top_node, rows_mask, cols_mask, boxes_mask, pos, 0);

                }

          }else{
              if(task_node->next != NULL){
                   top_node = task_node;
                    #pragma omp critical
                    {
                    task_node = task_node->next;
                    //sudoku[task_node->pos] =  0;
                    }
                    update_elem_mask(rows_mask, cols_mask, boxes_mask, top_node);

              }else{
                  top_node->pos = v_size;
                  done = 0;
                }
          }
}
/*//printf("t: %d pos: %d, val: %d\n",omp_get_thread_num(),top_node->pos, top_node->val );
while(task_node != NULL && task_node->val != 0){
    printf("%d\n",task_node->pos );
    printf("t: %d pos: %d, val: %d\n",omp_get_thread_num(),task_node->pos, task_node->val );
    sudoku[task_node->pos] = task_node->val;
    task_node = task_node->next;

}*/
}//single pragma

}//parallel pragma

    free(rows_mask);
    free(cols_mask);
    free(boxes_mask);

    return done;
}

struct node* add_node(struct node* n, int *r_mask, int *c_mask, int *b_mask, int pos, int val){
    int row = ROW(pos), col = COL(pos), box = BOX(row, col);
    struct node *temp;
    temp = (struct node*) calloc(1, sizeof(struct node));

    temp->r_mask = r_mask[row];
    temp->c_mask = c_mask[col];
    temp->b_mask = b_mask[box];
    temp->pos = pos;
    temp->val = val;

    if(n == NULL)
        temp->next = NULL;
    else
        temp->next = n;

    return temp;
}

void update_elem_mask(int *r_mask, int *c_mask, int *b_mask, struct node* n){
    int row = ROW(n->pos), col = COL(n->pos), box = BOX(row, col);

    r_mask[row] = n->r_mask;
    c_mask[col] = n->c_mask;
    b_mask[box] = n->b_mask;
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

            update_masks(sudoku[i], row, col, rows_mask, cols_mask, boxes_mask);
        }
    }
}

void rm_num_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int num_mask = int_to_mask(num);
    rows_mask[row] ^= num_mask;
    cols_mask[col] ^= num_mask;
    boxes_mask[BOX(row, col)] ^= num_mask;
}

void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask) {
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
void print_mask(int*mask){
  int i;
  for(i = 0; i < m_size; i++){
    printf("%d ",mask[i]);
  }
  printf("\n" );
}
