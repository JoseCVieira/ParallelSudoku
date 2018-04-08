#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <omp.h>
#include <time.h>
#include "list.h"

#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define ROW(i) i/m_size
#define COL(i) i%m_size
#define BOX(row, col) r_size*(row/r_size)+col/r_size

int r_size, m_size, v_size;


int* read_matrix(char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int* sudoku, int* cp_sudoku, int *rows_mask, int*cols_mask, int*boxes_mask, int pos, int val);
int* solve_from( int start_pos, int start_num, int* sudoku, int* cp_sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
int new_mask( int size);
int int_to_mask(int num);
void init_masks(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
int is_safe_num( int* rows_mask, int* cols_mask, int* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
void print_sudoku(int* sudoku);
void update_elem_mask(int *r_mask, int *c_mask, int *b_mask, ListNode *n);
void print_mask(int*mask);
Item build_item(int pos, int val,int* r_mask,int* c_mask,int *b_mask);
void clear_item(Item *this);
int main(int argc, char *argv[]) {
    clock_t start, end;
    int result, cell = 0, fst_pos= 0, pos = 0;
    int* sudoku, *rows_mask, *cols_mask, *boxes_mask, *cp_sudoku;
    double time;

    if(argc == 2){
        sudoku = read_matrix(argv);



        printf("\n\ninitial sudoku:");
        print_sudoku(sudoku);

        //start measurement
        start = clock();

        rows_mask = (int*) malloc(m_size * sizeof(int));
        cols_mask = (int*) malloc(m_size * sizeof(int));
        boxes_mask = (int*) malloc(m_size * sizeof(int));
        cp_sudoku = (int*) malloc(v_size * sizeof(int));
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
        init_masks(sudoku, rows_mask, cols_mask, boxes_mask);

        result = solve(sudoku,cp_sudoku,rows_mask, cols_mask, boxes_mask, 0, 0);

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

int solve(int* sudoku, int *cp_sudoku, int * rows_mask, int *cols_mask, int* boxes_mask,int  val,int pos){
    int row, col,last_val = 0,done = 1;


    if(done !=0)
    while(pos < v_size){
      //print_sudoku(sudoku);
        row  = ROW(pos);
        col = COL(pos);
         if(val < m_size){

              val++;
             if(is_safe_num(rows_mask, cols_mask, boxes_mask, row, col,val)){
               /*printf("saved mask for- pos: %d val: %d\n", pos, val);
               print_mask(rows_mask);*/
               //is safe so update masks
               update_masks(val, row, col, rows_mask, cols_mask, boxes_mask);
               //print_mask(rows_mask);
               //insert value in sudoku
               sudoku[pos]  =val;
               last_val = val;
               //searches next pos
               while(cp_sudoku[++pos] == -1);
               //goes deeper
               done = solve(sudoku,cp_sudoku, rows_mask, cols_mask, boxes_mask, 0, pos);
               if(done)
                return 1;
               while(cp_sudoku[--pos] == -1);
               rm_num_masks(last_val, row, col, rows_mask, cols_mask, boxes_mask);
             }

          }else{
                return 0;
          }

        }


}

/*struct node* add_node(struct node* n, int *r_mask, int *c_mask, int *b_mask, int pos, int val){
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
}*/


Item build_item(int pos, int val,int *r_mask,int *c_mask,int *b_mask){
  Item this;
  int i;
  this.pos = pos;
  this.val = val;
  //printf("malloc\n" );
  this.r_mask = (int *) malloc(m_size*sizeof(int));
  this.c_mask = (int *) malloc(m_size*sizeof(int));
  this.b_mask = (int *) malloc(m_size*sizeof(int));

  for(i = 0; i < m_size; i++){
    this.r_mask[i] = r_mask[i];
    this.c_mask[i] = c_mask[i];
    this.b_mask[i] = b_mask[i];
  }

  return this;
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

    int* sudoku = (int*)calloc(v_size, sizeof(int));

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
void clear_item(Item *this){
  free(this->r_mask);
  free(this->c_mask);
  free(this->b_mask);
}
