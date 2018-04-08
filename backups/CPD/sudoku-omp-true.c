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
int solve(int* sudoku);
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
    int cell, pos, val = 0, fst_pos = 0, done = 1,row, col;
    int* rows_mask;
    int* cols_mask;
    int* boxes_mask;
    List *private_list = init_list();
    List *shared_list = init_list();
    ListNode current_node;
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
    init_masks(sudoku, rows_mask, cols_mask, boxes_mask);

    insert_head(private_list, build_item(0, 0, rows_mask, cols_mask, boxes_mask));
    insert_head(shared_list, build_item(0, 0, rows_mask, cols_mask, boxes_mask));
      current_node.this = pop_head(private_list);
    #pragma omp parallel firstprivate(rows_mask, cols_mask, boxes_mask, current_node) shared(shared_list)
    {


    #pragma omp single nowait
    {
    while(current_node.this.pos < v_size){
    /*  printf("while-------%d\n", current_node.this.pos );
      print_mask(rows_mask);
      print_mask(cols_mask);
      print_mask(boxes_mask);
      printf("-----------\n" );*/
        done = 1;
      //  printf(" pos: %d, val: %d\n", current_node.this.pos, current_node.this.val);
    //  printf("tid: %d -pos:%d -val: %d\n",omp_get_thread_num(),current_node.this.pos, current_node.this.val);
         if(current_node.this.val < m_size){

              val = ++current_node.this.val;
              pos = current_node.this.pos;
              row = ROW(pos);
              col = COL(pos);
             if(is_safe_num(rows_mask, cols_mask, boxes_mask, row, col,val)){
                #pragma omp task firstprivate(rows_mask, cols_mask, boxes_mask, current_node)
                {

                  #pragma omp critical
                  {
                  //  printf("tid: %d -pos:%d -val: %d\n",omp_get_thread_num(),shared_list->head->this.pos, shared_list->head->this.val);

                    insert_head(shared_list, build_item(current_node.this.pos, current_node.this.val, rows_mask, cols_mask, boxes_mask));
                      update_masks(val, row, col, rows_mask, cols_mask, boxes_mask);
                      sudoku[pos]  =val;

                  }
                    insert_head(private_list, current_node.this);

                    while(cp_sudoku[++pos] == -1);
                    row = ROW(pos);
                    col = COL(pos);
                  }
                  #pragma omp taskwait
                    current_node.this = build_item(pos, 0, rows_mask, cols_mask, boxes_mask);
                    insert_head(private_list, current_node.this);


                }
          }else{
              //printf("HEY\n" );
              if(private_list->head->next != NULL && shared_list->head->next != NULL){
                //  printf("ITS ME\n" );
                    current_node.this = pop_head(private_list);
                    #pragma omp critical
                    {
                    current_node.this = pop_head(shared_list);
                    memcpy(rows_mask, current_node.this.r_mask, m_size*sizeof(int));
                    memcpy(cols_mask, current_node.this.c_mask, m_size*sizeof(int));
                    memcpy(boxes_mask, current_node.this.b_mask, m_size*sizeof(int));
                    }


              }else{
                  current_node.this.pos = v_size;
                  done = 0;
                }
          }

        }

}//single

}//parallel
    /*free(rows_mask);
    free(cols_mask);
    free(boxes_mask);*/

    return done;
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

void update_elem_mask(int *r_mask, int *c_mask, int *b_mask,ListNode *n){
    int row = ROW(n->this.pos), col = COL(n->this.pos), box = BOX(row, col), i;
    n->this.r_mask = (int *) malloc(m_size*sizeof(int));
    n->this.c_mask = (int *) malloc(m_size*sizeof(int));
    n->this.b_mask = (int *) malloc(m_size*sizeof(int));

    for(i = 0; i < m_size; i++){

      r_mask[i] = n->this.r_mask[i];
      c_mask[i] = n->this.c_mask[i];
      b_mask[i] = n->this.b_mask[i];
    }
}
Item build_item(int pos, int val,int *r_mask,int *c_mask,int *b_mask){
  Item this;
  int i;
  this.pos = pos;
  this.val = val;

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
