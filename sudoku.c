#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 9
#define UNASSIGNED 0

typedef struct{
    int **arr;
}t_array;

int is_exist_row(int **grid, int row, int num){
    int col;

    for (col = 0; col < 9; col++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

int is_exist_col(int **grid, int col, int num) {
    int row;

    for (row = 0; row < 9; row++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

int is_exist_box(int **grid, int startRow, int startCol, int num) {
    int row, col;

    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            if (grid[row + startRow][col + startCol] == num)
                return 1;
    return 0;
}

int is_safe_num(int **grid, int row, int col, int num) {
    return !is_exist_row(grid, row, num) && !is_exist_col(grid, col, num) && !is_exist_box(grid, row - (row % 3), col - (col %3), num);
}

int find_unassigned(int **grid, int *row, int *col) {
    for (*row = 0; *row < N; (*row)++)
        for (*col = 0; *col < N; (*col)++)
            if (grid[*row][*col] == 0)
                return 1;
    return 0;
}

int solve(int **grid) {

    double result;

    int row = 0, col = 0, num;

    if (!find_unassigned(grid, &row, &col))
        return 1;

    for (num = 1; num <= N; num++ )
        if (is_safe_num(grid, row, col, num)) {
            grid[row][col] = num;

            if (solve(grid))
                return 1;

            grid[row][col] = UNASSIGNED;
        }


    return 0;
}

void print_grid(int **grid) {
    int row, col;

    printf("\n+-------+-------+-------+\n");
    for (row = 0; row < N; row++) {
        if(row == 3 || row == 6)
            printf("+-------+-------+-------+\n|");
        else
            printf("|");

        for (col = 0; col < N; col++){
            if(col == 3 || col == 6)
                printf(" |");
            printf("%2d", grid[row][col]);
        }
        printf(" |");
        printf("\n");
    }
    printf("+-------+-------+-------+\n\n");
}

int check_row_box(int **grid, int row, int num) {
    int cont = 0;
    
    if(((row+1) % 3) == 0){
        cont += is_exist_row(grid, row-1, num);
        cont += is_exist_row(grid, row-2, num);
    }else if(((row+1) % 2) == 0){
        cont += is_exist_row(grid, row-1, num);
        cont += is_exist_row(grid, row+1, num);
    }else{
        cont += is_exist_row(grid, row+1, num);
        cont += is_exist_row(grid, row+2, num);
    }
    
    return cont;
}

int check_col_box(int **grid, int col, int num) {
    int cont = 0;
    
    if(((col+1) % 3) == 0){
        cont += is_exist_col(grid, col-1, num);
        cont += is_exist_col(grid, col-2, num);
    }else if(((col+1) % 2) == 0){
        cont += is_exist_col(grid, col-1, num);
        cont += is_exist_col(grid, col+1, num);
    }else{
        cont += is_exist_col(grid, col+1, num);
        cont += is_exist_col(grid, col+2, num);
    }
    
    return cont;
}

int verify_sudoku(int **grid){
    int i, j, row, col, num;

    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++){
            
            num = grid[i][j];
            
            if(num != 0){
            
                // row
                for (col = 0; col < 9; col++)
                    if (grid[i][col] == num && row != i && col != j){
                        printf("\n2row=%d | col=%d | num=%d", i, j, num);
                        return 0;
                    }
                
                // col
                for (row = 0; row < 9; row++)
                    if (grid[row][j] == num && row != i && col !=j){
                        printf("\n1row=%d | col=%d | num=%d", i, j, num);
                        return 0;
                    }
                
                // box
               /* for (row = 0; row < 3; row++)
                    for (col = 0; col < 3; col++)
                        if (grid[row + (i - (i % 3))][col + (j - (j %3))] == num  && row != i && col != j){
                            printf("\n3row=%d | col=%d | num=%d", i, j, num);
                            return 0;
                        }*/
                        
                /*is_exist_box(int grid[N][N], int startRow, int startCol, int num)
                is_exist_box(grid, row - (row % 3), col - (col %3)
                
                for (row = 0; row < 3; row++)
                    for (col = 0; col < 3; col++)
                        if (grid[row + startRow][col + startCol] == num)
                            return 1;
                return 0;*/
            }
        }
    }
    
    return 1;
}

void certain_elements(int **grid) {
    int i, j, k, check_row, check_col;
    char changed = 0;
    
    while(1){
        changed = 0;
        
        for(i = 0; i < N; i++){
            for(j = 0; j < N; j++){
                if(!grid[i][j]){
                    for(k = 0; k < N; k++){
                        check_row = 0;
                        check_col = 0;
                        if(is_safe_num(grid, i, j, k)){
                            
                            check_row = check_row_box(grid, i, k);
                            check_col = check_col_box(grid, j, k);
                            
                            if(check_row + check_col == 3){
                                if(check_row == 2){
                                    if(((j+1) % 3) == 0){
                                        if(grid[i][j-1] && grid[i][j-2]){
                                           grid[i][j] = k;
                                           changed = 1;
                                           printf("changed [100%%] i:%d j:%d, k:%d\n", i, j, k);
                                        }
                                    }else if(((j+1) % 2) == 0){
                                        if(grid[i][j-1] && grid[i][j+1]){
                                           grid[i][j] = k;
                                           changed = 1;
                                           printf("changed [100%%] i:%d j:%d, k:%d\n", i, j, k);
                                        }
                                    }else{
                                        if(grid[i][j+1] && grid[i][j+2]){
                                           grid[i][j] = k;
                                           changed = 1;
                                           printf("changed [100%%] i:%d j:%d, k:%d\n", i, j, k);
                                        }
                                    }
                                }else{
                                    if(((i+1) % 3) == 0){
                                        if(grid[i-1][j] && grid[i-2][j]){
                                           grid[i][j] = k;
                                           changed = 1;
                                           printf("changed [100%%] i:%d j:%d, k:%d\n", i, j, k);
                                        }
                                    }else if(((i+1) % 2) == 0){
                                        if(grid[i-1][j] && grid[i+1][j]){
                                           grid[i][j] = k;
                                           changed = 1;
                                           printf("changed [100%%] i:%d j:%d, k:%d\n", i, j, k);
                                        }
                                    }else{
                                        if(grid[i+1][j] && grid[i+2][j]){
                                           grid[i][j] = k;
                                           changed = 1;
                                           printf("changed [100%%] i:%d j:%d, k:%d\n", i, j, k);
                                        }
                                    }
                                }
                            }else if(check_row + check_col == 4){
                                grid[i][j] = k;
                                changed = 1;
                                           printf("changed [100%%] i:%d j:%d, k:%d\n", i, j, k);
                            }
                        }
                    }
                    
                }
            }
        }
        
        if(!changed)
            break;
    }    
    
}

int read_matrix(t_array *grid, int argc, char *argv[]){
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    int i, j, k;
    int rooted_matrix_size, n_matrix_size;
    char *line = NULL;

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
    }else{
        fprintf(stderr, "%s opened\n",argv[1]);
    }
    }
    if(read = getline(&line, &len, fp) != -1){
    rooted_matrix_size = atoi(line);
    }
    n_matrix_size = rooted_matrix_size *rooted_matrix_size;

    grid->arr = (int **)malloc(n_matrix_size * sizeof(int*));
    for(i = 0; i < n_matrix_size; i++)
        grid->arr[i] = (int*) malloc(n_matrix_size * sizeof(int));

    for(i = 0; getline(&line, &len, fp) != -1; i++){
    k = 0;
    for (j = 0; line[j] != '\n'; j++) {

        if(line[j] == ' '){
        line[j] = '\0';
        k++;
        }else{
        grid->arr[i][k] = (int)line[j]-'0';
        printf("%d ", grid->arr[i][k]);
        }
    }
    printf("\n" );
    }
    //  printf("%d \n", &grid);
    //free(grid);
    fclose(fp);
    return rooted_matrix_size;
}

int main(int argc, char *argv[]) {
    clock_t start, end;
    int flag;
    
    int n_matrix_size, i, j; //size of matrix
    int rooted_matrix_size;//size of Sudoku box

    t_array grid1;
    rooted_matrix_size = read_matrix(&grid1, argc, argv);
    n_matrix_size = rooted_matrix_size*rooted_matrix_size;

    printf("Initial grid:\n");
    print_grid(grid1.arr);
    printf("Final grid:\n");
    start = clock();
    certain_elements(grid1.arr);
    end = clock();
    print_grid(grid1.arr);
    flag = verify_sudoku(grid1.arr);
    
    if(flag)
        printf("\nRigth!\n\n");
    else
        printf("\nWrong!\n\n");
    
    /*if (solve(grid)){
        end = clock();
        print_grid(grid);
    }else{
        end = clock();
        printf("no solution");
    }*/
      
    printf("%lf\n\n",(double) (end-start)/CLOCKS_PER_SEC);
    return 0;
}
