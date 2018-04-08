#include <stdio.h>
#include <stdlib.h>
#include "list.h"

int r_size, m_size, v_size;

List * init_list(void){

	List* newList = (List*)malloc(sizeof(List));
	newList -> head = NULL;
	newList -> tail = NULL;

	return newList;
}

void free_list( List* list){

	ListNode* next;
	ListNode* aux;

	for(aux = list -> head; aux != NULL; aux = next){

		next = aux -> next;
		free( aux );
	}
	free(list);

	return;
}

ListNode * newNode( ListNode* next, ListNode* prev, Item this){

	ListNode* new_node = (ListNode*) malloc(sizeof(ListNode));

	if(new_node == NULL)
		return NULL;

	new_node -> this = this;
	new_node -> next = next;
	new_node -> prev = prev;

	return new_node;
}


void insert_head( List* list, Item this){

	if( list -> head == NULL){ //the list is empty
		list -> tail = newNode(NULL, NULL, this);
		list -> head = list -> tail;
		return;
	}
	list -> head -> prev = newNode(list -> head, NULL, this);
	list -> head = list -> head -> prev;
	return;
}

void insert_tail( List* list, Item this ){

	if( list -> tail == NULL){ //the list is empty
		list -> tail = newNode(NULL, NULL, this);
		list -> head = list -> tail;
		return;
	}

	list -> tail -> next = newNode(NULL, list->tail,  this);
	list -> tail = list -> tail -> next;
	return;
}

Item pop_head(List* list){

	Item item = list -> head -> this;

	ListNode* aux = list -> head -> next;
	if(aux == NULL){
		free(list->head);
		list->head = NULL;
		list->tail = NULL;
	 	return item;
	}
	aux -> prev = NULL;
	free(list->head);
	list -> head = aux;
	return item;
}

Item pop_tail(List* list){

	Item item = list -> tail -> this;
	ListNode* aux = list -> tail -> prev;
	if(aux ==  NULL){
	 	//free(list->tail);
	 	return item;
	}
	aux -> next = NULL;
	free(list -> tail);
	list -> tail = aux;
	return item;
}


void pop_all( List* list){
	ListNode* aux;

	aux = list->head;
	while(aux != NULL){
		list -> head = list -> head -> next;
		free(aux);
		aux = list -> head;
	}
}

int listSize(List* list){

	ListNode* aux;
	int counter;
	if(list->head == NULL) return 0;
	counter = 0;
	for(aux = list->head; aux != NULL; aux = aux->next)
		counter++;

	return counter;
}
