#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "string.h"

#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define ROW(i) i/m_size
#define COL(i) i%m_size

struct node{
  int iteration, val;
  struct node *next;
};


int r_size, m_size, v_size;
struct node* add_node(struct node*, int, int);
int* read_matrix(char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int* sudoku);
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


        //free(sudoku);
    }else
        printf("invalid input arguments.\n");

    return 0;
}



int solve(int* sudoku) {
  int cell, pos = 0, col, row, num = 0, safe = 0, direction =1, i ,j;

  /*variables used on solve*/
  int cp_sudoku[v_size];
  /*Allocating masks*/
  int* rows_mask = (int*) malloc(m_size * sizeof(int));
  int* cols_mask = (int*) malloc(m_size * sizeof(int));
  int* boxes_mask = (int*) malloc(m_size * sizeof(int));

  /*masks initialization*/
  init_masks(sudoku, rows_mask, cols_mask, boxes_mask);

  for(cell = 0; cell < v_size; cell++){
      if(sudoku[cell])
          cp_sudoku[cell] = UNCHANGEABLE;
      else
          cp_sudoku[cell] = UNASSIGNED;
  }
  //Starts searching loop
  while(pos <v_size){
    //print_sudoku(sudoku);
    //reads row and col
    row = ROW(pos);
    col = COL(pos);
    //printf("%d, %d, %d\n", row, col, cp_sudoku[pos]);
    //increments cp_sudoku val

    //verifies if cp_sudoku val is within bounds
    if(cp_sudoku[pos]>=0 && cp_sudoku[pos]< m_size){
      cp_sudoku[pos]++;
      /*if(cp_sudoku[pos] == 10)
        printf("AQUI");*/
      if(safe = is_safe_num(rows_mask, cols_mask, boxes_mask, row, col, cp_sudoku[pos])){
        direction = 1;
        rm_num_masks(sudoku[pos], row,col, rows_mask, cols_mask, boxes_mask);
        sudoku[pos] = cp_sudoku[pos];
        update_masks(cp_sudoku[pos], row,col, rows_mask, cols_mask, boxes_mask);
        //printf("%d\n", pos);
      }else{
        if(cp_sudoku[pos] >= m_size){
          if(pos == -1){
            return 0;
          }else{
            direction = -1;
            //update_masks(cp_sudoku[pos], row,col, rows_mask, cols_mask, boxes_mask);

            rm_num_masks(sudoku[pos], row,col, rows_mask, cols_mask, boxes_mask);
            sudoku[pos] = 0;
            cp_sudoku[pos] = 0;
            pos+=direction;
          }
        }
      }
      if(safe){

        pos+=direction;
      }
    }else{
      if(cp_sudoku[pos] >= 0){
        rm_num_masks(sudoku[pos], row,col, rows_mask, cols_mask, boxes_mask);
        sudoku[pos] = 0;
        cp_sudoku[pos] = 0;
      }
      pos+=direction;
    }

  }

  free(rows_mask);
  free(cols_mask);
  free(boxes_mask);
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
    if(num >0)
      return (1 << (num-1));
    else
      return 0;
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

            update_masks(sudoku[i], row,col, rows_mask, cols_mask, boxes_mask);
        }
    }
}

void rm_num_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int num_mask = int_to_mask(num);
    //printf("%d\n",num_mask );
    rows_mask[row] ^= num_mask;
    cols_mask[col] ^= num_mask;
    boxes_mask[r_size*(row/r_size)+col/r_size] ^=num_mask;
}

void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int new_mask = int_to_mask(num);
    rows_mask[row] |= new_mask;
    cols_mask[col] |= new_mask;
    boxes_mask[r_size*(row/r_size)+col/r_size] |= new_mask;

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
