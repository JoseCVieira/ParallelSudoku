#include "sudoku-serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define UNASSIGNED 0
#define S_INIT 0
#define S_FINAL 1
#define S_INV 2
#define S_FULL 3

typedef struct{
    int **arr;
} t_array;

int r_size, m_size;
int cont_test = 0;

int read_matrix(t_array *grid, int argc, char *argv[]){
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    int i, j, k, l, aux2;
    int rooted_matrix_size, n_matrix_size;
    char *line = NULL, aux[3];

    //verifies if the program calling has at least one argument
    if (argc<1) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        exit(1);
    }

    //if it has, opens the file
    if (argc > 1) {
        fprintf(stderr, "opening file %s\n", argv[1]);

        //verifies if the file was correctly opened
        if ((fp = fopen(argv[1], "r+")) == NULL) {
            fprintf(stderr, "unable to open file %s\n", argv[1]);
            exit(1);
        }else
            fprintf(stderr, "%s opened\n",argv[1]);
    }

    if(read = getline(&line, &len, fp) != -1)
        rooted_matrix_size = atoi(line);

    n_matrix_size = rooted_matrix_size *rooted_matrix_size;

    grid->arr = (int **)malloc(n_matrix_size * sizeof(int*));

    for(i = 0; i < n_matrix_size; i++)
        grid->arr[i] = (int*) malloc(n_matrix_size * sizeof(int));

    for(i = 0; getline(&line, &len, fp) != -1; i++){
        k = 0;
        l = 0;
        for (j = 0; line[j] != '\n'; j++) {
            if(line[j] == ' ' || line[j] == '\n'){
                aux[l] = '\0';
                grid->arr[i][k] = atoi(aux);
                l = 0;
                k++;
            }else{
                aux[l] = line[j];
                l++;
            }
        }
    }

    fclose(fp);
    return rooted_matrix_size;
}

int is_exist_row(int **grid, int row, int num){
    int col;

    for (col = 0; col < m_size; col++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

int is_exist_col(int **grid, int col, int num) {
    int row;

    for (row = 0; row < m_size; row++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

int is_exist_box(int **grid, int startRow, int startCol, int num) {
    int row, col;

    for (row = 0; row < r_size; row++)
        for (col = 0; col < r_size; col++)
            if (grid[row + startRow][col + startCol] == num)
                return 1;
    return 0;
}

int is_safe_num(int **grid, int row, int col, int num) {
    return !is_exist_row(grid, row, num) && !is_exist_col(grid, col, num) && !is_exist_box(grid, row - (row % r_size), col - (col %r_size), num);
}

/*int find_unassigned(int **grid, int *row, int *col) {
    for (*row = 0; *row < m_size; (*row)++)
        for (*col = 0; *col < m_size; (*col)++)
            if (grid[*row][*col] == 0)
                return 1;
    return 0;
}*/

int solve(int **grid) {
    int r= 0, c= 0, last_r = 0, last_c = 0, i  =1, inv_val = 0, j;
    int count;
    double result;
    int temp_matrix[10];
    int **row_stack;



    row_stack = (int **) malloc(m_size*sizeof(int*));
    for (i = 0; i < m_size; i++) {
      row_stack[i] = (int *) malloc((S_FULL+1)*sizeof(int*));
      for(j = 0; j < m_size; j++)
        row_stack[i][j] = 0;
        row_stack[i][S_FINAL] = m_size;
        row_stack[i][S_INIT] = -1;
        row_stack[i][S_INV] = 0;
    }
    r = 0;
    while(r < m_size){
      //is row full?
      if(1){
        c = 0;
        while(c < m_size){
          //is box empty?
          if(grid[r][c] == 0){
            i = 1;
            while(i <=m_size){
              if(!is_exist_row(grid, r, i) &&  !is_exist_col(grid, c, i)){
                grid[r][c] = i;
                last_c = c;
                last_r = r;
                break;
              }else{
                i++;
              }

              for(j = 0; j < m_size; j++){
                if(grid[r][j] == 0){
                  grid[r][j] = 0;
                  last_c--;
                  break;
                }

              }

            }//val loop
          }//if grid
          print_grid(grid, r_size, m_size);
          printf("%d---------------------------------------------\n", c );
          c++;
        }//col loop
      }//row full?
      r++;
    }//row loop
      /*print_grid(grid, r_size, m_size);
      printf("---------------------------------------------\n" );*/
  //  printf("%d to %d\n", row_stack[i][S_INIT], row_stack[i][S_FINAL]);




}


/* final print function */

/*void print_grid(int **grid) {
    int row, col;

    for (row = 0; row < m_size; row++) {
        for (col = 0; col < m_size; col++)
            printf("%d ", grid[row][col]);
        printf("\n");
    }
}*/

int main(int argc, char *argv[]) {
    clock_t start, end, result;
    t_array grid1;

    if(argc == 2){
        printf("\n");
        r_size = read_matrix(&grid1, argc, argv);
        m_size = r_size * r_size;

        printf("\ninitial sudoku:");
        print_grid(grid1.arr, r_size, m_size);
        printf("\n\n\nnumber of zeros:%d\n\n", nr_zeros(grid1.arr, m_size));
        printf("result sudoku:");

        //start measurement
        start = clock();

        result = solve(grid1.arr);

        // end measurement
        end = clock();

        print_grid(grid1.arr, r_size, m_size);

        // mesmo que nao esteja completo verifica se esta correto (apenas para testes)
        verify_sudoku(grid1.arr, m_size, r_size) == 1 ? printf("rigth!\n") : printf("wrong!\n");

        // verifica se tem solucao ou nao
        result == 1 ? printf("solved!\n") : printf("no solution!\n");

        printf("it took %lf sec.\n\n",(double) (end-start)/CLOCKS_PER_SEC);
    }else
        printf("invalid input arguments.\n");

    return 0;
}
