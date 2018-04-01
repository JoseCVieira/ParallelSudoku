#include <stdlib.h>
#include <string.h>
#include "stack.h"


typedef struct StackNode {
	StackItem item;				/** The data of this node. **/
	struct StackNode *next;		/** The next node (the one below the top). **/
} StackNode;

struct Stack {
	size_t count; 	/** The number of items in the stack. **/
	StackNode *top;	/** The top item of the stack. **/
};



Stack *stackCreate()
{
	/* Create a stack and set everything to the default values. */
	Stack *stack = (Stack *) malloc(sizeof *stack);
	if(stack == NULL)
		return NULL;
	
	stack->count = 0;
	stack->top = NULL;
	
	return stack;
}

void stackDestroy(Stack *stack)
{
	stackClean(stack);
	free(stack);
}

void stackClean(Stack *stack)
{
	while(!stackIsEmpty(stack))
		stackPop(stack);
}

bool stackIsEmpty(Stack *stack)
{
	return stack->top == NULL ? true : false;
}

size_t stackSize(Stack *stack)
{
	return stack->count;
}

StackItem stackTop(Stack *stack)
{
	return stack->top->item;
}

bool stackPush(Stack *stack, StackItem item)
{
	StackNode *newNode = (StackNode *) malloc(sizeof *newNode);
	if(newNode == NULL)
		return false;
	
	newNode->item = item;
	newNode->next = stack->top;
	stack->top = newNode;
	
	stack->count += 1;
	return true;
}

StackItem stackPop(Stack *stack)
{
	StackNode *oldTop;
	StackItem item;
	
	if(stack->top == NULL)
		return 0; /** @todo Make a better way to return this error. **/
	
	oldTop = stack->top;
	item = oldTop->item;
	stack->top = oldTop->next;
	free(oldTop);
	oldTop = NULL;
	
	stack->count -= 1;
	return item;
}
