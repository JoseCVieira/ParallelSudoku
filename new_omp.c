#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define UNASSIGNED 0
#define ROW 0
#define COL 1
#define BOX 2
#define VAL 2

typedef struct{
    int **arr;
} t_array;

int r_size, m_size;

int read_matrix(t_array *grid, int argc, char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int **grid, int m_zeros, int* rows_mask, int* cols_mask, int* boxes_mask);
int new_mask( int size);
int int_to_mask(int num);
int init_masks(int** grid, int* rows_mask, int* cols_mask, int* boxes_mask);
void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);

int main(int argc, char *argv[]) {
    clock_t start, end;
    t_array grid1;
    int i, j, result, nr_zeros;

    if(argc == 2){
        printf("\n");
        r_size = read_matrix(&grid1, argc, argv);
        m_size = r_size * r_size;

        int* rows_mask = (int*) malloc(m_size * sizeof(int));
        int* cols_mask = (int*) malloc(m_size * sizeof(int));
        int* boxes_mask = (int*) malloc(m_size * sizeof(int));

        printf("\ninitial sudoku:");
        print_grid(grid1.arr, r_size, m_size);
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
    }else
        printf("invalid input arguments.\n");

    return 0;
}



int solve(int **grid, int m_zeros, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int i, j, val, max, zeros = 1, flag_back = 0, i_aux, index=0, row, col;
    int vector[m_size * m_size];
    int mask_copy[m_zeros][3];
    
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
        zeros = 1;
        
        if(flag_back){
            flag_back = 0;
            
            // clear element
            vector[i_aux] = 0;
            
            // search and increment nearest element(on their left)
            for(i = i_aux - 1; i >= 0; i--){
                row = i/m_size;
                col = i%m_size;
                
                
                if((vector[i] > 0) && (vector[i] < m_size)){
                    vector[i]++;
                    grid[row][col]++;
                    
                    printf("index=%d, row=%d, col=%d, val=%d\n", index, row, col, vector[i]);
                    
                    print_grid(grid, r_size, m_size);
                    
                    mask_copy[index][ROW] = rows_mask[row];
                    mask_copy[index][COL] = cols_mask[col];
                    mask_copy[index][BOX] = boxes_mask[r_size*(row/r_size)+col/r_size];
                    
                    i=-2;
                    
                    update_masks(vector[i], row, col, rows_mask, cols_mask, boxes_mask);
                    
                    break;
                }else if(vector[i] == m_size){
                    vector[i] = 0;
                    grid[row][col] = 0;
                    
                    index --;
                    
                    rows_mask[row] = mask_copy[index][ROW];
                    cols_mask[col] = mask_copy[index][COL];
                    boxes_mask[r_size*(row/r_size)+col/r_size] = mask_copy[index][BOX];
                    
                    zeros ++;
                }
            }
            
            if(i == -1)
                return 0; //impossible
        }
        
        printf("\nsaiu\n");
        
        for(i = 0; i < max; i++){
            row = i/m_size;
            col = i%m_size;
                
            if(!vector[i]){
                zeros ++;
                for(val = 1; val <= m_size; val++){
                    
                    if(is_safe_num( rows_mask, cols_mask, boxes_mask, i/m_size, i%m_size, val)){
                        vector[i] = val;
                        grid[row][col] = val;
                        
                        //printf("index=%d, row=%d, col=%d, val=%d\n", index, row, col, val);
                        
                        for(j = 0; j < m_size; j++)
                        printf("%d ", rows_mask[j]);
                        printf("\n");
                        
                        for(j = 0; j < m_size; j++)
                            printf("%d ", cols_mask[j]);
                        printf("\n");
                        
                        for(j = 0; j < m_size; j++)
                            printf("%d ", boxes_mask[j]);
                        printf("\n");
                        
                        mask_copy[index][ROW] = rows_mask[row];
                        mask_copy[index][COL] = cols_mask[col];
                        mask_copy[index][BOX] = boxes_mask[r_size*(row/r_size)+col/r_size];
                        
                        index ++;
                        
                        update_masks(val, row, col, rows_mask, cols_mask, boxes_mask);
                        
                        print_grid(grid, r_size, m_size);
                        
                        sleep(1);
                        
                        break;
                    }else if(val == m_size){
                        printf("index=%d, row=%d, col=%d, val=%d\n", index, row, col, val);
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

void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask) {
    int new_mask = int_to_mask(num);
    rows_mask[row] = rows_mask[row] | new_mask;
    cols_mask[col] = cols_mask[col] | new_mask;
    boxes_mask[r_size*(row/r_size)+col/r_size] = boxes_mask[r_size*(row/r_size)+col/r_size] | new_mask;
}

int read_matrix(t_array *grid, int argc, char *argv[]) {
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
