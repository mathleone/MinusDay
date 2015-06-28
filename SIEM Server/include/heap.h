#ifndef __HEAP__H__
#define __HEAP__H__

#include <stdio.h>
#include <stdlib.h>

#define NP_CHECK(ptr) \
    { \
        if (NULL == (ptr)) { \
            fprintf(stderr, "%s:%d NULL POINTER: %s \n", \
                __FILE__, __LINE__, #ptr); \
            exit(-1); \
        } \
    } \

#define DEBUG(msg) fprintf(stderr, "%s:%d %s", __FILE__, __LINE__, (msg))

typedef struct PQueue_s {
    size_t size;
    size_t capacity;
    void **data;
    int (*cmp)(const void *d1, const void *d2);
} PQueue;
PQueue *pqueue_new(int (*cmp)(const void *d1, const void *d2), size_t capacity);
void pqueue_delete(PQueue *q);
void pqueue_enqueue(PQueue *q, const void *data);
void *pqueue_dequeue(PQueue *q);

#endif