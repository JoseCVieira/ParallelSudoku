#include "list.h"

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

Item pop_tail(List *list) {
    Item item;

    ListNode *node = list->tail;
  
    if (--list->len)
        (list->tail = node->prev)->next = NULL;
    else
        list->tail = list->head = NULL;

    node->next = node->prev = NULL;

    item = node-> this;
    free(node);
    return item;
}

void print_list(List* list){
    ListNode *aux;

    aux = list->head;
    while(aux!=NULL){
        printf("(%d,%d) ", aux->this.cell, aux->this.num);
        aux = aux->next;
    }
    printf("\n");
    return;
}
