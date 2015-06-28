#include "heap.h"

#define LEFT(x) (2 * (x) + 1)
#define RIGHT(x) (2 * (x) + 2)
#define PARENT(x) ((x) / 2)

void pqueue_maxheapify(PQueue *q, size_t idx);

PQueue *pqueue_new(int (*cmp)(const void *d1, const void *d2), size_t capacity) {
    PQueue *res = malloc(sizeof(*res));
    res->cmp = cmp;
    res->data = malloc(capacity * sizeof(*(res->data)));
    res->capacity = capacity;
    res->size = 0;
    return res;
}
void pqueue_delete(PQueue *q) {
    if (NULL != q) {
        return;
    }
    free(q->data);
    free(q);
}
int pqueue_isEmpty(PQueue *q){
    return !(q->size);
}
int pqueue_isFull(PQueue *q){
    return (q->size == q->capacity);
}
void pqueue_maxenqueue(PQueue *q, const void *data) {
    size_t i;
    void *tmp = NULL;
    NP_CHECK(q);
    if (q->size >= q->capacity) {
        return;
    }
    q->data[q->size] = (void*) data;
    i = q->size;
    q->size++;
    while(i > 0 && q->cmp(q->data[i], q->data[PARENT(i)]) > 0) {
        tmp = q->data[i];
        q->data[i] = q->data[PARENT(i)];
        q->data[PARENT(i)] = tmp;
        i = PARENT(i);
    }
}
void pqueue_minenqueue(PQueue *q, const void *data) {
    size_t i;
    void *tmp = NULL;
    NP_CHECK(q);
    if (q->size >= q->capacity) {
        return;
    }
    q->data[q->size] = (void*) data;
    i = q->size;
    q->size++;
    while(i > 0 && q->cmp(q->data[i], q->data[PARENT(i)]) < 0) {
        tmp = q->data[i];
        q->data[i] = q->data[PARENT(i)];
        q->data[PARENT(i)] = tmp;
        i = PARENT(i);
    }
}
void pqueue_minheapify(PQueue *q, size_t idx) {
    void *tmp = NULL;
    size_t l_idx, r_idx, smlr_idx;
    NP_CHECK(q);

    l_idx = LEFT(idx);
    r_idx = RIGHT(idx);

    if (l_idx < q->size && q->cmp(q->data[l_idx], q->data[idx]) < 0) {
        smlr_idx = l_idx;
    } else {
        smlr_idx = idx;
    }

    if (r_idx < q->size && q->cmp(q->data[r_idx], q->data[smlr_idx]) < 0) {
        smlr_idx = r_idx;
    }

    if (smlr_idx != idx) {
        tmp = q->data[smlr_idx];
        q->data[smlr_idx] = q->data[idx];
        q->data[idx] = tmp;
        pqueue_minheapify(q, smlr_idx);
    }
}
void *pqueue_mindequeue(PQueue *q) {
    void *data = NULL;
    NP_CHECK(q);
    if (q->size < 1) {         
         return NULL;     
    }     
    data = q->data[0];
    q->data[0] = q->data[q->size-1];
    q->size--;
    pqueue_minheapify(q, 0);
    return (data);
}
void *pqueue_maxdequeue(PQueue *q) {
    void *data = NULL;
    NP_CHECK(q);
    if (q->size < 1) {         
         return NULL;     
    }     
    data = q->data[0];
    q->data[0] = q->data[q->size-1];
    q->size--;
    pqueue_maxheapify(q, 0);
    return (data);
}
void pqueue_maxheapify(PQueue *q, size_t idx) {
    void *tmp = NULL;
    size_t l_idx, r_idx, lrg_idx;
    NP_CHECK(q);

    l_idx = LEFT(idx);
    r_idx = RIGHT(idx);

    if (l_idx < q->size && q->cmp(q->data[l_idx], q->data[idx]) > 0) {
        lrg_idx = l_idx;
    } else {
        lrg_idx = idx;
    }

    if (r_idx < q->size && q->cmp(q->data[r_idx], q->data[lrg_idx]) > 0) {
        lrg_idx = r_idx;
    }

    if (lrg_idx != idx) {
        tmp = q->data[lrg_idx];
        q->data[lrg_idx] = q->data[idx];
        q->data[idx] = tmp;
        pqueue_maxheapify(q, lrg_idx);
    }
}
int pqueueSize(PQueue *q){
    return q->size;
}