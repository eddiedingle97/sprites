#ifndef __PQUEUE_H__
#define __PQUEUE_H__

struct pq
{
    float *keys;
    void **p;
    int noitems;
    int size;
};

struct pq *pq_create(int startsize);
void pq_destroy(struct pq *pq);
void pq_insert(struct pq *pq, float key, void *p);
void *pq_pop(struct pq *pq);

#endif