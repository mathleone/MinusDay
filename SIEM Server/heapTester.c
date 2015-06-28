#include <stdio.h>
#include <stdlib.h>
#include <heap.h>

typedef struct test_s {
    int priority;
    char *log;
} Test;

Test *test_new(int priority) {
    Test *t = NULL;
    t = malloc(sizeof(*t));
    NP_CHECK(t);
    t->priority = priority;
    t->log = "234mtrhrtngre";
    printf("Inserindo %d - %s\n", t->priority, t->log);
    return (t);
}

void test_delete(Test *t) {
    if (NULL != t) {
        free(t);
    }
}

int test_compare(const void *d1, const void *d2) {
    return ((Test*)d1)->priority - ((Test*)d2)->priority;
}

int main(int argc, char *argv[]) {
    PQueue *q = NULL;
    Test *t = NULL;
    int i;

    srand(time(NULL));

    /* A priority Queue containing a maximum of 10 elements */
    q = pqueue_new(test_compare, 10);

    for(i = 0; i < 10; ++i) {
        /* Adding elements to priority Queue */
        t = test_new(rand());
        pqueue_minenqueue(q, t);
    }

    for(i = 0; i < 10; ++i) {
    	Test * buffer = (Test*)pqueue_mindequeue(q);          
        printf("%d - %s\n", buffer->priority, buffer->log);
        /* Free memory - me lazy */
    }

    /* Free memory - me lazy */

    return (0);
}