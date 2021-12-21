#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

struct dict
{
    void **keys;
    void **p;
    int size;
    int (*comp)(void *, void *);
};

struct dict *dict_create(int (*comp)(void *, void *));
void dict_destroy(struct dict *dict);
void dict_add_entry(struct dict *dict, void *key, void *p);
void *dict_get_entry(struct dict *dict, void *key);

#endif