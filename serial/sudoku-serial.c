#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#define UNASSIGNED 0
#define UNCHANGEABLE -1

#define ROW(i) i/m_size
#define COL(i) i%m_size
#define BOX(row, col) r_size*(row/r_size)+col/r_size

typedef struct{
    int cell;
    int num;
}Item;

typedef struct ListNode{
    Item this;
    struct ListNode *next;
    struct ListNode *prev;
}ListNode;

typedef struct{
    ListNode *head;
    ListNode *tail;
    int len;
}List;

int r_size, m_size, v_size; int nr_it = 0;

List * init_list(void);
ListNode* newNode(Item this);
void insert_head(List* list, Item this);
Item pop_head(List* list);

void update_masks(int num, int row, int col, uint64_t *rows_mask, uint64_t *cols_mask, uint64_t *boxes_mask);
int is_safe_num( uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num);
void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int solve_from(int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work, int last_pos);
void init_masks(int* sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask);
int exists_in( int index, uint64_t* mask, int num);
int* read_matrix(char *argv[]);
void print_sudoku(int *sudoku);
int int_to_mask(int num);
int new_mask( int size);
int solve(int *sudoku);

int main(int argc, char *argv[]){
    int* sudoku;

    if(argc == 2){
        sudoku = read_matrix(argv);

        if(solve(sudoku))
            print_sudoku(sudoku);
        else
            printf("No solution\n");
        
        printf("nr_it=%d\n", nr_it);
    }else
        printf("invalid input arguments.\n");
    
    free(sudoku);    

    return 0;
}

int solve(int* sudoku){
    int i, flag_start = 0, solved = 0, start_pos, start_num, last_pos;
    Item hyp;
    
    uint64_t *r_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    uint64_t *c_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    uint64_t *b_mask_array = (uint64_t*) malloc(m_size * sizeof(uint64_t));
    int      *cp_sudoku    = (int*)      malloc(v_size * sizeof(int));
    List *work = init_list();
    
    for(i = 0; i < v_size; i++) {
        if(sudoku[i])
            cp_sudoku[i] = UNCHANGEABLE;
        else{
            cp_sudoku[i] = UNASSIGNED;
            if(!flag_start){
                flag_start = 1;
                start_pos = i;
            }
            last_pos = i;
        }
    }
    
    init_masks(sudoku, r_mask_array, c_mask_array, b_mask_array);

    for(start_num = 1; start_num <= m_size; start_num++) {
        if(!solved){
            hyp.cell = start_pos;
            hyp.num = start_num;

            insert_head(work, hyp);
        
            if(solve_from(cp_sudoku, r_mask_array, c_mask_array, b_mask_array, work, last_pos)) {
                solved = 1;
                
                for(i = 0; i < v_size; i++)
                    if(cp_sudoku[i] != UNCHANGEABLE)
                        sudoku[i] = cp_sudoku[i];
            }
        }
    }

    free(work);
    free(r_mask_array);
    free(c_mask_array);
    free(b_mask_array);
    free(cp_sudoku);
    
    if(solved)
        return 1;
    return 0;
}

int solve_from(int* cp_sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, List* work, int last_pos) {
    int cell, val;
    Item hyp;
    
    hyp = pop_head(work);
    int start_pos = hyp.cell;

    if(!is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(hyp.cell), COL(hyp.cell), hyp.num))
        return 0;

    while(1){
        update_masks(hyp.num, ROW(hyp.cell), COL(hyp.cell), rows_mask, cols_mask, boxes_mask);
        cp_sudoku[hyp.cell] = hyp.num;
        
        nr_it ++;
        
        for(cell = hyp.cell + 1; cell < v_size; cell++){

            if(!cp_sudoku[cell]){
                //for(val = 1; val <= m_size; val++){
                for(val = m_size; val >= 1; val--){
                    //printf("val=%d\n", val);
                    
                    if(is_safe_num(rows_mask, cols_mask, boxes_mask, ROW(cell), COL(cell), val)){
                         if(cell == last_pos){
                            cp_sudoku[cell] = val;
                            return 1;
                         }
                        
                        hyp.cell = cell;
                        hyp.num = val;
                        insert_head(work, hyp);
                    }
                }
                //sleep(1);
                    
                if(work->head == NULL){
                    for(cell = v_size - 1; cell >= start_pos; cell--)
                        if(cp_sudoku[cell] > 0){
                            rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                            cp_sudoku[cell] = UNASSIGNED;
                        }
                    return 0;
                }else
                    break;
            }
        }
        
        hyp = pop_head(work);
        
        for(cell--; cell >= hyp.cell; cell--){
            if(cp_sudoku[cell] > 0) {
                rm_num_masks(cp_sudoku[cell],  ROW(cell), COL(cell), rows_mask, cols_mask, boxes_mask);
                cp_sudoku[cell] = UNASSIGNED;
            }
        }
    }
}

int exists_in(int index, uint64_t* mask, int num) {
    int res, masked_num = int_to_mask(num);

    res = mask[index] | masked_num;
    if(res != mask[index]) 
        return 0;
    return 1;
}

int is_safe_num(uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask, int row, int col, int num) {
    return !exists_in(row, rows_mask, num) && !exists_in(col, cols_mask, num) && !exists_in(BOX(row, col), boxes_mask, num);
}

int new_mask(int size) {
    return (0 << (size-1));
}

int int_to_mask(int num) {
    return (1 << (num-1));
}

void init_masks(int* sudoku, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
    int i, row, col;

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

void rm_num_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
    int num_mask = int_to_mask(num);
    rows_mask[row] ^= num_mask;
    cols_mask[col] ^= num_mask;
    boxes_mask[BOX(row, col)] ^= num_mask;
}

void update_masks(int num, int row, int col, uint64_t* rows_mask, uint64_t* cols_mask, uint64_t* boxes_mask) {
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
    
    for (i = 0; i < v_size; i++) {
        if(i%m_size != m_size - 1)
            printf("%2d ", sudoku[i]);
        else
            printf("%2d\n", sudoku[i]);
    }
}

List* init_list(void){
    List* newList = (List*)malloc(sizeof(List));
    newList -> head = NULL;
    newList -> tail = NULL;
    newList->len = 0;
    
    return newList;
}

ListNode* newNode(Item this){
    ListNode* new_node = (ListNode*) malloc(sizeof(ListNode));

    new_node -> this = this;
    new_node -> next = NULL;

    return new_node;
}

void insert_head(List* list, Item this){
    ListNode* node = newNode(this);
    
    if (list->len) {
        node->next = list->head;
        node->prev = NULL;
        list->head->prev = node;
        list->head = node;
    }else {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    }
    
    ++list->len;
}

Item pop_head(List* list){
    Item item;
    
    ListNode* node = list->head;

    if (--list->len)
        (list->head = node->next)->prev = NULL;
    else
        list->head = list->tail = NULL;

    node->next = node->prev = NULL;
    
    item = node -> this;
    free(node);
    return item;
}
