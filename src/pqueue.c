#include <stdio.h>
#include "pqueue.h"
#include "sprites.h"

static void pq_increase_size(struct pq *pq);

struct pq *pq_create(int startsize)
{
    struct pq *out = s_malloc(sizeof(struct pq), NULL);
    out->size = startsize;
    out->keys = s_malloc(out->size * sizeof(float), NULL);
    out->p = s_malloc(out->size * sizeof(void *), NULL);
    out->noitems = 0;

    return out;
}

void pq_destroy(struct pq *pq)
{
    s_free(pq->keys, NULL);
    s_free(pq->p, NULL);
    s_free(pq, NULL);
}

void pq_insert(struct pq *pq, float key, void *p)
{
    if(pq->noitems + 1 >= pq->size)
        pq_increase_size(pq);

    int i;
    for(i = pq->noitems - 1; i >= 0; i--)
    {
        if(key >= pq->keys[i])//putting >= causes FIFO ordering within keys, put > if LIFO within keys is desired
        {
            pq->keys[i + 1] = pq->keys[i];
            pq->p[i + 1] = pq->p[i];
        }

        else
            break;
    }

    pq->keys[i + 1] = key;
    pq->p[i + 1] = p;
    pq->noitems++;
}

static void pq_increase_size(struct pq *pq)
{
    pq->size *= 2;
    pq->keys = s_realloc(pq->keys, pq->size * sizeof(float), NULL);
    pq->p = s_realloc(pq->p, pq->size * sizeof(void *), NULL);
}

void *pq_pop(struct pq *pq)
{
    if(pq->noitems > 0)
        return pq->p[--pq->noitems];
    return NULL;
}