#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

void *initQueue(){
	QUEUE *q = malloc(sizeof(QUEUE));
	q->inicio = NULL;
	q->fim = NULL;
	return q;
}

int insertQueue(QUEUE *q, void *Data){
    queueNode *node = malloc(sizeof(queueNode));
    if(node == NULL)
        return 0;
    node->Node = Data;
    node->prox = NULL;
    if(isEmpty(q))
        q->inicio = node;
    else
        q->fim->prox = node;
    q->fim = node;
    q->size++;
    return 1;
}

void *removeQueue(QUEUE *q){
    void *data;
    queueNode *node;
    if(isEmpty(q))
        return NULL;
    node = q->inicio;
    data = node->Node;
    q->inicio = node->prox;
    free(node);
    q->size--;
    return data;
}

int isEmpty(QUEUE *q){
	return q->inicio == NULL;
}

void freeQueue(QUEUE *q){
    free(q);
}

int sizeofQueue(QUEUE *q){
    return q->size;
}