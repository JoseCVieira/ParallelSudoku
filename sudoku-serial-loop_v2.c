#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define UNASSIGNED 0
#define ROW 0
#define COL 1
#define BOX 2
#define VAL 2
#define STR_MAX 35
typedef struct{
    int **arr;
} t_array;

int r_size, m_size;

int read_matrix(t_array *grid, int argc, char *filename);
int exists_in( int index, int* mask, int num);
int solve(int **grid, int m_zeros, int* rows_mask, int* cols_mask, int* boxes_mask);
int new_mask( int size);
int int_to_mask(int num);
int init_masks(int** grid, int* rows_mask, int* cols_mask, int* boxes_mask);
void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
int is_safe_num( int* rows_mask, int* cols_mask, int* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);


int main(int argc, char *argv[]) {
    clock_t start, end;
    t_array grid1;
    int i, j, result, pid, status, n_solves, nr_zeros;
    int max_solves;

    char *filename, *url;

    //interpret inputs
    switch (argc) {
      case 2://program+input_file_name
        filename = argv[1];
        max_solves = 1;

        break;
      case 3://program +input_file_name+ loop times
        filename = argv[1];
        max_solves = atoi(argv[2]);
        break;
      case 4://same +url
        filename = argv[1];
        max_solves = atoi(argv[2]);
        url = argv[3];
        break;
      default:
        filename = (char*) malloc(sizeof(char)*(STR_MAX));
        sprintf(filename, "input_file");
        max_solves = 1;
        url = (char*) malloc(sizeof(char)*(STR_MAX));
        sprintf(url, "http://www.menneske.no/sudoku/eng/");
        break;
    }

    for(n_solves = 0; n_solves < max_solves; n_solves++){

      pid = fork();

      if(!pid){
        if(argc)
        execlp("python3", "python3", "sudoku_parser.py",url, (char*) NULL);
      }else{
        while(wait(&status) > 0);

            printf("\n");
            r_size = read_matrix(&grid1, argc, filename);
            m_size = r_size * r_size;

            int* rows_mask = (int*) malloc(m_size * sizeof(int));
            int* cols_mask = (int*) malloc(m_size * sizeof(int));
            int* boxes_mask = (int*) malloc(m_size * sizeof(int));

            printf("\ninitial sudoku:");
            print_grid(grid1.arr, r_size, m_size);
            printf("number of zeros:%d\n\n", nr_zeros);
            printf("result sudoku:");

            //start measurement
            start = clock();

            nr_zeros = init_masks(grid1.arr, rows_mask, cols_mask, boxes_mask);
            result = solve(grid1.arr, nr_zeros, rows_mask, cols_mask, boxes_mask);

            // end measurement
            end = clock();

            print_grid(grid1.arr, r_size, m_size);

            // mesmo que nao esteja completo verifica se esta correto (apenas para testes)
            verify_sudoku(grid1.arr, m_size, r_size) == 1 ? printf("rigth!\n") : printf("wrong!\n");

            // verifica se tem solucao ou nao
            result == 1 ? printf("solved!\n") : printf("no solution!\n");

            printf("it took %lf sec.\n\n",(double) (end-start)/CLOCKS_PER_SEC);
        }
    }
    return 0;
}

int solve(int **grid, int m_zeros, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int i, j, val, max, zeros = 1, flag_back = 0, i_aux, row, col, val_aux;
    int vector[m_size * m_size];

    // init vector
    for(i = 0; i < m_size; i++){
        for(j = 0; j < m_size; j++){
            if(grid[i][j])
                vector[i * m_size + j] = -1; // unchangeable
            else
                vector[i * m_size + j] = 0;
        }
    }

    max = m_size * m_size;

    while(zeros){
        zeros = 0;

        i = 0;
        if(flag_back){
            // search nearest element(on their left)
            for(i = i_aux - 1; i >= 0; i--){
                row = i/m_size;
                col = i%m_size;

                if((vector[i] > 0) && (vector[i] < m_size)){
                    val_aux = vector[i] + 1;
                    rm_num_masks(vector[i], row, col, rows_mask, cols_mask, boxes_mask);
                    vector[i] = 0;
                    grid[row][col] = 0;
                    break;
                }else if((vector[i] > 0) && vector[i] == m_size){
                    rm_num_masks(vector[i], row, col, rows_mask, cols_mask, boxes_mask);
                    vector[i] = 0;
                    grid[row][col] = 0;
                    zeros ++;
                }
            }

            if(i == -1)
                return 0; //impossible
        }

        for(; i < max; i++){
            row = i/m_size;
            col = i%m_size;

            if(!vector[i] || flag_back){

                val = 1;
                if(flag_back){
                    val = val_aux;
                    flag_back = 0;
                }
                zeros++;

                for(; val <= m_size; val++){
                    if(is_safe_num( rows_mask, cols_mask, boxes_mask, row, col, val)){
                        vector[i] = val;
                        grid[row][col] = val;

                        update_masks(val, row, col, rows_mask, cols_mask, boxes_mask);

                        //print_grid(grid, r_size, m_size);

                        break;
                    }else if(val == m_size){
                        flag_back = 1;
                        i_aux = i;
                        i = max; //break
                    }
                }
            }
        }
    }
    return 1;
}

int exists_in( int index, int* mask, int num) {

    int res;
    int masked_num = int_to_mask(num);

    res = mask[index] | masked_num;
    if( res != mask[index]) return 0;

    return 1;
}

int is_safe_num( int* rows_mask, int* cols_mask, int* boxes_mask, int row, int col, int num) {
    return !exists_in( row, rows_mask, num) && !exists_in( col, cols_mask, num) && !exists_in( r_size*(row/r_size)+col/r_size, boxes_mask, num);
}

int new_mask(int size) {
    return (0 << (size-1));
}

int int_to_mask(int num) {
    return (1 << (num-1));
}

int init_masks(int** grid, int* rows_mask, int* cols_mask, int* boxes_mask) {

    int mask, i, row, col, zeros = 0;
    #pragma omp parallel
    {
        #pragma omp for private(i)
        for(i = 0; i < m_size; i++){
            rows_mask[i] = 0;
            cols_mask[i] = 0;
            boxes_mask[i] = 0;
        }

        #pragma omp for private(row, col, mask) reduction(+:zeros)
        for(row = 0; row < m_size; row++){
            for(col = 0; col < m_size; col++){

                //if the cell has a number add that number to the current row, col and box mask
                if(grid[row][col]){
                    mask = int_to_mask( grid[row][col] ); //convert number found to mask ex: if dim=4x4, 3 = 0010
                    rows_mask[row] = rows_mask[row] | mask; //to add the new number to the current row's mask use bitwise OR

                    #pragma omp atomic
                    cols_mask[col] = cols_mask[col] | mask;
                    boxes_mask[r_size*(row/r_size)+col/r_size] = boxes_mask[r_size*(row/r_size)+col/r_size] | mask;
                }else
                    zeros++;
            }
        }

    }
    return zeros;
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

int read_matrix(t_array *grid, int argc, char *filename){
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    int i, j, k, l, aux2;
    int rooted_matrix_size, n_matrix_size;
    char *line = NULL, aux[3];

    //if it has, opens the file

    fprintf(stderr, "opening file %s\n", filename);

        //verifies if the file was correctly opened
    if ((fp = fopen(filename, "r+")) == NULL) {
        fprintf(stderr, "unable to open file %s\n", filename);
        exit(1);
    }else
        fprintf(stderr, "%s opened\n",filename);


    if( (read = getline(&line, &len, fp)) != -1)
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