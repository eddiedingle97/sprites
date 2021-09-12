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

void graph_destroy_with_function(struct graph *graph, void (*func)(struct vertex *))
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
    v->name = name;
    v->mark = 0;
    v->val = 0;

    return v;
}

struct edge *graph_add_edge(struct graph *graph, int source, int dest, int weight)
{
    if(source >= graph->novertices || dest >= graph->novertices)
    {
        fprintf(stderr, "referenced invalid vertex index %d", source > dest ? source : dest);
        return NULL;
    }

    graph->edges = s_realloc(graph->edges, ++graph->noedges * sizeof(struct edge), "graph_add_edge: graph->edges");
    struct edge *edge = &graph->edges[graph->noedges - 1];
    edge->from = source;
    edge->to = dest;
    edge->weight = weight;
    struct vertex *s = &graph->vertices[source];
    s->edges = s_realloc(s->edges, ++s->noedges * sizeof(int), "graph_add_edge: s->edges");
    s->edges[s->noedges - 1] = graph->noedges - 1;
    struct edge *out = edge;

    if(!(graph->type & DIRECTED))
    {
        graph->edges = s_realloc(graph->edges, ++graph->noedges * sizeof(struct edge), "graph_add_edge: graph->edges");
        struct edge *edge = &graph->edges[graph->noedges - 1];
        edge->to = source;
        edge->from = dest;
        edge->weight = weight;
        struct vertex *d = &graph->vertices[dest];
        d->edges = s_realloc(d->edges, ++d->noedges * sizeof(int), "graph_add_edge: d->edges");
        d->edges[s->noedges - 1] = graph->noedges - 1;
    }

    return out;
}

struct edge *graph_get_edge(struct graph *graph, struct vertex *vertex, int i)
{
    if(i < vertex->noedges)
        return &graph->edges[vertex->edges[i]];
    return NULL;
}

struct vertex *graph_get_vertex(struct graph *graph, int i)
{
    if(i < graph->novertices)
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

    //printf("%s\n", v->name);

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
    else
        printf("no vertices to search\n");
}

void graph_unmark(struct graph *graph)
{
    int i;
    for(i = 0; i < graph->novertices; i++)
        graph->vertices[i].mark = 0;
    graph->marked = 0;
}

int graph_is_connected(struct graph *graph)
{
    if(graph->marked)
        graph_unmark(graph);
    graph_dfs(graph);
    int i, c = 0;
    for(i = 0; i < graph->novertices; i++)
    {
        c += graph->vertices[i].mark;
        graph->vertices[i].mark = 0;
    }
    graph->marked = 0;

    return c == graph->novertices;
}