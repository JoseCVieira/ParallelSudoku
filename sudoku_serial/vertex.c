#include <stdlib.h>
#include "vertex.h"

Vertex* new_vertex(int num, int cell){

    Vertex *new_v;
    new_v = malloc(sizeof(Vertex));
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

int get_next_v(int curr_cell, int *vect){
      
    int aux = curr_cell+1;
    while( vect[aux] == UNCHANGEABLE && aux <= (v_size-2)) aux++;
    if(vect[aux] != UNCHANGEABLE) 
        return aux;
    else 
        return curr_cell;
}

int get_prev_v(int curr_cell, int prev_cell, int *vect ){

    prev_cell -= prev_cell - curr_cell;
    curr_cell = prev_cell;
   // while(vect[cell_id] == UNCHANGEABLE) cell_id++;
    return curr_cell;
}

