#include "sudoku-aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "stack.h"

#define UNASSIGNED 0
#define UNCHANGEABLE -1
#define FALSE 0
#define TRUE 1
#define ROW(i) i/m_size
#define COL(i) i%m_size

typedef struct Vertex {
    int num;         
    int cell; 
    int visited;  
} Vertex;

int r_size, m_size, v_size;


int* read_matrix(char *argv[]);
int exists_in( int index, int* mask, int num);
int solve(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
int new_mask( int size);
int int_to_mask(int num);
void init_masks(int* sudoku, int* rows_mask, int* cols_mask, int* boxes_mask);
void update_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
int is_safe_num( int* rows_mask, int* cols_mask, int* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, int* rows_mask, int* cols_mask, int* boxes_mask);
void print_sudoku(int* sudoku);

Vertex* new_vertex(int num, int cell_id);
int vertex_visited(Vertex** vertex);


int main(int argc, char *argv[]) {
    clock_t start, end;
    int result;
    double time;
    int* sudoku;

    if(argc == 2){
        sudoku = read_matrix(argv);

        int* rows_mask = (int*) malloc(m_size * sizeof(int));
        int* cols_mask = (int*) malloc(m_size * sizeof(int));
        int* boxes_mask = (int*) malloc(m_size * sizeof(int));

        printf("\n\ninitial sudoku:");
        print_sudoku(sudoku);

        //start measurement
        start = clock();

        init_masks(sudoku, rows_mask, cols_mask, boxes_mask);        
        result = solve(sudoku, rows_mask, cols_mask, boxes_mask);

        // end measurement
        end = clock();

        printf("result sudoku:");
        print_sudoku(sudoku);
        
        verify_sudoku(sudoku, r_size) == 1 ? printf("corret, ") : printf("wrong, ");
        result == 1 ? printf("solved!\n") : printf("no solution!\n");

        time = (double) (end-start)/CLOCKS_PER_SEC;
        printf("took %lf sec (%.3lf ms).\n\n", time, time*1000.0);

        free(rows_mask);
        free(cols_mask);
        free(boxes_mask);
        free(sudoku);        
    }else
        printf("invalid input arguments.\n");
    
    return 0;
}

Vertex* new_vertex(int num, int cell){

    Vertex *new_v;
    new_v = (Vertex*)malloc(sizeof(Vertex));
    new_v->num = num;
    new_v->cell = cell;
    new_v->visited = FALSE;

    return new_v;
}

int vertex_visited(Vertex **vertex){

    if( (*vertex)->visited == TRUE ) 
        return TRUE;
    
    (*vertex)->visited = TRUE;

    return FALSE;
}


int solve(int *sudoku, int *rows_mask, int *cols_mask, int *boxes_mask) {
    
    // *******************************
    // ******************************* mudar o cell_id++ para contar com as que já estão preenchidas
    int i, num, root_num, cell_id = 0, aux, prev_cell_id = -1;
    int cp_sudoku[v_size];
    Stack *dfs_stack = NULL;
    Vertex *vertex = NULL, *prev_vertex = NULL;

    // init cp_sudoku
    for(i = 0; i < v_size; i++){
        if(sudoku[i])
            cp_sudoku[i] = UNCHANGEABLE;
        else
            cp_sudoku[i] = UNASSIGNED;
    }

    //start the search on the first cell of the sudoku
    for( root_num = 1; root_num < m_size; root_num++ ){

        
	//find first empty cell
	while(cp_sudoku[cell_id] == UNCHANGEABLE) cell_id++;	

        //new_vertex
        Vertex* vertex = new_vertex(root_num, cell_id);

	//init  search stack
    	dfs_stack = stackCreate();

        //push first vertex
        aux = stackPush(dfs_stack, (StackItem)vertex);

        while(!stackIsEmpty(dfs_stack)){

	    prev_vertex = vertex;
            vertex = stackPop(dfs_stack);
	

	    if( prev_vertex->cell > vertex->cell ){
                sudoku[prev_vertex->cell] = 0; //if the cell has been visited with no solution found we go back one cell
                rm_num_masks(prev_vertex->num,  ROW(prev_vertex->cell), COL(prev_vertex->cell), rows_mask, cols_mask, boxes_mask);
                cell_id -= prev_cell_id - vertex->cell;
                while(cp_sudoku[cell_id] == UNCHANGEABLE) cell_id++;
            }

            sudoku[cell_id] = vertex->num;
	    update_masks( vertex->num, ROW(cell_id), COL(cell_id), rows_mask, cols_mask, boxes_mask);
	    print_sudoku(sudoku);
	    
	    if(vertex->visited == TRUE) continue;

            if( cell_id == (v_size-1) ){
                
		if(verify_sudoku(sudoku, r_size)){
		    printf("SUCCESS!!!\n");
                    print_sudoku(sudoku);
                    return 1;
		}
                else continue; 
            }
	   // if(vertex->visited == TRUE) continue; 
            
	    cell_id++; //if there are new cells to explore we will advance one cell
            while(cp_sudoku[cell_id] == UNCHANGEABLE && cell_id < (v_size-1)) cell_id++;
            

            for( num = m_size; num >= 1; num--){

                vertex = new_vertex(num, cell_id);

                if(vertex->visited == FALSE)    
                    stackPush(dfs_stack, vertex);
            }
        }
        printf("NO SOLUTION\n");
        return 0;

    }
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
            
            mask = int_to_mask(sudoku[i]);          //convert number found to mask ex: if dim=4x4, 3 = 0010
            rows_mask[row] = rows_mask[row] | mask; //to add the new number to the current row's mask use bitwise OR
            cols_mask[col] = cols_mask[col] | mask;
            boxes_mask[r_size*(row/r_size)+col/r_size] = boxes_mask[r_size*(row/r_size)+col/r_size] | mask;
        }
    }
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

int* read_matrix(char *argv[]) {
    FILE *fp;
    size_t characters, len = 1;
    char *line = NULL, aux[2];
    int i, j, k;
    
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

    k = 0;
    len = m_size * 2;
    for(i = 0; (characters = getline(&line, &len, fp)) != -1; i++){
        for (j = 0; j < characters; j++) {
            if(isdigit(line[j])){
                aux[0] = line[j];
                aux[1] = '\0';
                sudoku[k++] = atoi(aux);
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
