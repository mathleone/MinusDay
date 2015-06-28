#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

typedef struct queueNode{
	void *Node;
	struct queueNode *prox;
}queueNode;

typedef struct QUEUE{
	queueNode *inicio;
	queueNode *fim;
	int size;
}QUEUE;

void *initQueue();

int insertQueue(QUEUE *, void *);

void *removeQueue(QUEUE *);

int isEmpty(QUEUE *);

void freeQueue(QUEUE *);

int sizeofQueue(QUEUE *);

#endif