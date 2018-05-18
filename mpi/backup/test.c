#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../list.h"

int main(void){
    
    List *work = init_list();
    Item item;
    int i;
    
    for(i = 0; i < 20; i++){
        item.cell = i;
        item.num = i;
        insert_head(work, item);
        print_list(work);
    }
    
    /*for(i = 0; i < 40; i++){
        if(work->head != NULL){
            item = pop_head(work);
            print_list(work);
        }
    }*/
    
    for(i = 20; i >= 0; i--){
        item.cell = i;
        item.num = i;
        list_remove(work, item);
        print_list(work);
    }
    
    return 0;
}
