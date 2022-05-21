#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "sprites.h"

struct graph *graph_create(int type)
{
    struct graph *out = s_malloc(sizeof(struct graph), "graph_create");
    out->vertices = NULL;
    out->novertices = 0;
    out->edges = NULL;
    out->noedges = 0;
    out->type = type;
    out->marked = 0;
    return out;
}

void graph_destroy(struct graph *graph)
{
    if(graph)
    {
        int i;
        for(i = 0; i < graph->novertices; i++)
        {
            if(graph->vertices[i].edges)
                s_free(graph->vertices[i].edges, NULL);
        }
        if(graph->vertices)
            s_free(graph->vertices, NULL);
        if(graph->edges)
            s_free(graph->edges, NULL);
        s_free(graph, "freeing graph");
    }
}

void graph_destroy_with_function(struct graph *graph, void (*func)(void *))
{
    if(graph)
    {
        int i;
        for(i = 0; i < graph->novertices; i++)
        {
            if(graph->vertices[i].edges)
            {
                s_free(graph->vertices[i].edges, NULL);
                func(graph->vertices[i].p);
            }
        }
        if(graph->vertices)
            s_free(graph->vertices, NULL);
        if(graph->edges)
            s_free(graph->edges, NULL);
        s_free(graph, "freeing graph");
    }
}

struct vertex *graph_add_vertex(struct graph *graph, void *p, char *name)
{
    graph->vertices = s_realloc(graph->vertices, ++graph->novertices * sizeof(struct vertex), "graph_add_vertex");
    struct vertex *v = &graph->vertices[graph->novertices - 1];
    v->edges = NULL;
    v->noedges = 0;
    v->p = p;
    #ifdef DEBUG
        v->name = name;
    #endif
    v->mark = 0;
    v->val = 0;

    return v;
}

struct edge *graph_add_edge_v(struct graph *graph, struct vertex *source, struct vertex *dest, int weight)
{
    return graph_add_edge(graph, source - graph->vertices, dest - graph->vertices, weight);
}

struct edge *graph_add_edge(struct graph *graph, int source, int dest, int weight)
{
    if(source < 0 || source >= graph->novertices)
    {
        #ifdef DEBUG
            fprintf(stderr, "referenced invalid vertex index %d", source);
        #endif
        return NULL;
    }
    if(dest < 0 || dest >= graph->novertices)
    {
        #ifdef DEBUG
            fprintf(stderr, "referenced invalid vertex index %d", dest);
        #endif
        return NULL;
    }

    graph->edges = s_realloc(graph->edges, ++graph->noedges * sizeof(struct edge), NULL);
    struct edge *edge = graph->edges + graph->noedges - 1;
    edge->from = source;
    edge->to = dest;
    edge->weight = weight;
    struct vertex *s = &graph->vertices[source];
    s->edges = s_realloc(s->edges, ++s->noedges * sizeof(int), NULL);
    s->edges[s->noedges - 1] = graph->noedges - 1;
    struct edge *out = edge;

    if(!(graph->type & DIRECTED))
    {
        graph->edges = s_realloc(graph->edges, ++graph->noedges * sizeof(struct edge), NULL);
        struct edge *edge = graph->edges + graph->noedges - 1;
        edge->to = source;
        edge->from = dest;
        edge->weight = weight;
        struct vertex *d = &graph->vertices[dest];
        d->edges = s_realloc(d->edges, ++d->noedges * sizeof(int), NULL);
        d->edges[d->noedges - 1] = graph->noedges - 1;
    }

    return out;
}

void graph_remove_edge(struct graph *graph, struct edge *edge)
{
    if(!edge)
        return;
    //printf("removing edge %d %d\n", edge->to, edge->from);
    struct vertex *from = graph_get_vertex(graph, edge->from);
    int i;
    if(from)
    {
        for(i = 0; i < from->noedges; i++)
        {
            if(graph_get_edge(graph, from, i) == edge)
            {
                from->edges[i] = -1;
                break;
            }
        }
    }
    if(!(DIRECTED & graph->type))
    {
        struct vertex *to = graph_get_vertex(graph, edge->to);
        if(to)
        {
            for(i = 0; i < to->noedges; i++)
            {
                struct edge *other = graph_get_edge(graph, to, i);
                if(other && other->from == edge->to && other->to == edge->from && other->weight == edge->weight)
                {
                    to->edges[i] = -1;
                    other->to = -1;
                    other->from = -1;
                    break;
                }
            }
        }
    }
    edge->to = -1;
    edge->from = -1;
}

struct edge *graph_get_edge(struct graph *graph, struct vertex *vertex, int i)
{
    if(i >= 0 && i < vertex->noedges && vertex->edges[i] >= 0 && vertex->edges[i] < graph->noedges)
        return &graph->edges[vertex->edges[i]];
    return NULL;
}

struct vertex *graph_get_vertex(struct graph *graph, int i)
{
    if(i >= 0 && i < graph->novertices)
        return &graph->vertices[i];
    return NULL;
}

struct vertex *graph_get_next_vertex(struct graph *graph, struct vertex *vertex, int i)
{
    struct edge *e = graph_get_edge(graph, vertex, i);
    if(e)
        return graph_get_vertex(graph, e->to);
    return NULL;
}

static void dfs(struct graph *graph, struct vertex *v)
{
    if(!v)
        return;
    if(v->mark)
        return;

    #ifdef DEBUG
        printf("%s\n", v->name);
    #endif

    int i;
    v->mark = 1;
    for(i = 0; i < v->noedges; i++)
    {
        dfs(graph, graph_get_next_vertex(graph, v, i));
    }
}

void graph_dfs(struct graph *graph)
{
    if(graph->novertices)
    {
        graph->marked = 1;
        dfs(graph, &graph->vertices[0]);
    }
}

int graph_unmark(struct graph *graph)
{
    int i, count = 0;
    for(i = 0; i < graph->novertices; i++)
    {
        graph->vertices[i].mark = 0;
        count++;
    }
    graph->marked = 0;
    return count;
}

int graph_is_connected(struct graph *graph)
{
    if(graph->marked)
        graph_unmark(graph);
    graph_dfs(graph);

    return graph->novertices == graph_unmark(graph);
}