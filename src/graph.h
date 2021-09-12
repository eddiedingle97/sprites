#ifndef __GRAPH_H__
#define __GRAPH_H__

struct edge
{
    int to;
    int from;
    int weight;
};

struct vertex
{
    int *edges;
    void *p;
    char *name;
    int noedges;
    char mark;
    int val;
};

struct graph
{
    struct vertex *vertices;
    int novertices;
    struct edge *edges;
    int noedges;
    char type;
    char marked;
};

enum GRAPHTYPE {DIRECTED = 1, CYCLIC = 2};

struct graph *graph_create(int type);
void graph_destroy(struct graph *graph);
void graph_destroy_with_function(struct graph *graph, void (*)(struct vertex *));
struct vertex *graph_add_vertex(struct graph *graph, void *p, char *name);
struct edge *graph_add_edge(struct graph *graph, int source, int dest, int weight);
struct edge *graph_get_edge(struct graph *graph, struct vertex *vertex, int i);
struct vertex *graph_get_vertex(struct graph *graph, int i);
struct vertex *graph_get_next_vertex(struct graph *graph, struct vertex *vertex, int i);
void graph_dfs(struct graph *graph);
void graph_unmark(struct graph *graph);
int graph_is_connected(struct graph *graph);

#endif