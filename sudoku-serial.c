/*
* Sudoku solver- serial version CPD
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 9
#define UNASSIGNED 0

typedef struct{
  int **arr;
}t_array;

int read_matrix(t_array *, int, char *[]);
int is_exist_row(int **, int, int);
int is_exist_col(int **, int, int);
int is_exist_box(int **, int, int, int);
int is_safe_num(int **, int, int, int);
int find_unassigned(int **, int*, int*);
int solve(int **);
void print_grid(int **, int);


int main(int argc, char *argv[]) {
  int n_matrix_size, i, j; //size of matrix
  int rooted_matrix_size;//size of Sudoku box

    /*int grid[N][N] = {{4,0,0, 0,0,1, 0,0,5},
                      {0,0,1, 0,3,0, 0,0,0},
                      {3,0,0, 2,9,0, 0,0,0},
                      {0,9,3, 0,0,7, 8,4,0},
                      {0,0,0, 0,5,0, 0,7,0},
                      {0,6,0, 1,0,0, 0,0,0},
                      {0,8,5, 0,0,0, 9,0,0},
                      {7,0,0, 5,0,0, 0,0,0},
                      {0,0,0, 0,4,2, 0,0,0}};*/
    t_array grid1;
    rooted_matrix_size = read_matrix(&grid1, argc, argv);
    n_matrix_size = rooted_matrix_size*rooted_matrix_size;
    printf("N = %d \t Rooted N = %d\n",n_matrix_size, rooted_matrix_size);
    print_grid(grid1.arr, n_matrix_size);

    /*clock_t start, end;
    start = clock();
    if (solve(grid)){
        end = clock();
        print_grid(grid);
    }else{
        end = clock();
        printf("no solution");
      }

    printf("%f",(double) (end-start)/CLOCKS_PER_SEC);*/
    return 0;
}
/*
*
*/
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
/*
*
*/
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

void print_grid(int **grid, int n_size) {
    int row, col;

    for(row = 0; row < n_size; row++){
      for(col = 0; col < n_size; col++)
        printf("%d ",grid[row][col] );
      printf("\n");
    }
}
