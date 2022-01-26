#include <stdio.h>
#include "sprites.h"
#include "dictionary.h"

static void dict_shuffle_up(struct dict *dict, int i);

struct dict *dict_create(int (*comp)(void *, void *))
{
    struct dict *out = s_malloc(sizeof(struct dict), NULL);
    out->keys = NULL;
    out->p = NULL;
    out->size = 0;
    out->comp = comp;

    return out;
}

void dict_destroy(struct dict *dict)
{
    if(dict->keys)
        s_free(dict->keys, NULL);
    if(dict->p)
        s_free(dict->p, NULL);
    s_free(dict, NULL);
}

void dict_add_entry(struct dict *dict, void *key, void *p)
{
    if(dict->comp)
    {
        int i;
        
        for(i = 0; i < dict->size; i++)
        {
            if(dict->comp(dict->keys[i], key) == 0)
            {
                dict->p[i] = p;
                return;
            }

            else if(dict->comp(dict->keys[i], key) > 0)
            {
                dict->keys = s_realloc(dict->keys, ++dict->size * sizeof(void *), NULL);
                dict->p = s_realloc(dict->p, dict->size * sizeof(void *), NULL);
                dict_shuffle_up(dict, i);
                dict->keys[i] = key;
                dict->p[i] = p;
                return;
            }
        }
        if(i == dict->size)
        {
            dict->keys = s_realloc(dict->keys, ++dict->size * sizeof(void *), NULL);
            dict->p = s_realloc(dict->p, dict->size * sizeof(void *), NULL);
            dict->keys[i] = key;
            dict->p[i] = p;
        }
    }
    else
    {
        int i;

        for(i = 0; i < dict->size; i++)
        {
            if(dict->keys[i] == key)
            {
                dict->p[i] = p;
                return;
            }

            else if(dict->keys[i] > key)
            {
                dict->keys = s_realloc(dict->keys, ++dict->size * sizeof(void *), "dict->keys: dict_add_entry");
                dict->p = s_realloc(dict->p, dict->size * sizeof(void *), "dict->p: dict_add_entry");
                dict_shuffle_up(dict, i);
                dict->keys[i] = key;
                dict->p[i] = p;
                return;
            }
        }
        if(i == dict->size)
        {
            dict->keys = s_realloc(dict->keys, ++dict->size * sizeof(void *), "dict->keys: dict_add_entry");
            dict->p = s_realloc(dict->p, dict->size * sizeof(void *), "dict->p: dict_add_entry");
            dict->keys[i] = key;
            dict->p[i] = p;
        }
    }
}

void *dict_get_entry(struct dict *dict, void *key)
{
    if(dict->comp)
    {
        int r = dict->size - 1, l = 0, m;
        while(l <= r)
        {
            m = l + (r - l) / 2;

            if(dict->comp(key, dict->keys[m]) < 0)
                r = m - 1;
                
            else if(dict->comp(key, dict->keys[m]) > 0)
                l = m + 1;

            else if(dict->comp(key, dict->keys[m]) == 0)
                return dict->p[m];
        }
    }
    else
    {
        int r = dict->size - 1, l = 0, m;
        while(l <= r)
        {
            m = l + (r - l) / 2;

            if(key < dict->keys[m])
                r = m - 1;
                
            else if(key > dict->keys[m])
                l = m + 1;

            else if(key == dict->keys[m])
                return dict->p[m];
        }
    }
    
    return NULL;
}

static void dict_shuffle_up(struct dict *dict, int i)
{
    int j;
    for(j = dict->size - 2; j >= i ; j--)
    {
        dict->keys[j + 1] = dict->keys[j];
        dict->p[j + 1] = dict->p[j];
    }
}