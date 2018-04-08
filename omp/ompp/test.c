#include <stdlib.h>
#include <stdio.h>
#include "list.h"


int main(void){
	int i,j;
	Item pair;
	List* list = init_list();

	for( i = 0; i < 2 ; i++){
		for(int j = 0; j < 2; j++){
			pair.cell = i;
			pair.num = j;
			insert_head( list, pair);
		}
	}

	while( (list->head) != NULL){
		
		pair = pop_tail(list);

	}

	free_list(list);
	/*for(i=0;i<9;i++){
		for(j=0;j<9;j++){
			printf("row: %d, col: %d, box: %d\n",i,j,3*(i/3)+j/3);
		}
	} */
	return 0;
}
