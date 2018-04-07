#include "item.h"

typedef struct ListNode{
	Item this;
	struct ListNode *next;
	struct ListNode *prev;
}ListNode;

typedef struct{
	ListNode *head;
	ListNode *tail;
}List;


List * init_list(void);
void free_list( List* list);
ListNode * newNode( ListNode* next, ListNode* prev, Item this);
void insert_head( List* list, Item this);
void insert_tail( List* list, Item this );
Item pop_head(List* list);
Item pop_tail(List* list);
void pop_all( List* list);
int listSize(List* list);