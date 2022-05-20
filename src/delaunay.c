#include <stdio.h>
#include <stdlib.h>
#include "sprites.h"
#include "emath.h"
#include "graph.h"

static int (*get_x)(void *);
static int (*get_y)(void *);
static void delaunay_triangulation_f(struct vertex *vertices, int size, struct graph *graph);
static int inside_circumcircle(void *one, void *two, void *three, void *four);
static int get_angle(struct vertex *one, struct vertex *two, struct vertex *three);
static struct vertex *get_candidate(struct graph *graph, struct vertex *target, struct vertex *neighbor, int ccw);
static int intersect(struct graph *graph, struct vertex *one, struct vertex *two, struct edge **inter);

static void delaunay_triangulation_f(struct vertex *vertices, int size, struct graph *graph)
{
    if(size == 2)
    {
        //printf("base case (%d, %d), (%d, %d)\n", get_x(vertices[0].p), get_y(vertices[0].p), get_x(vertices[1].p), get_y(vertices[1].p));
        graph_add_edge_v(graph, &vertices[0], &vertices[1], 0);
        return;
    }

    if(size == 3)
    {
        //printf("base case (%d, %d), (%d, %d), (%d, %d)\n", get_x(vertices[0].p), get_y(vertices[0].p), get_x(vertices[1].p), get_y(vertices[1].p), get_x(vertices[2].p), get_y(vertices[2].p));
        graph_add_edge_v(graph, &vertices[0], &vertices[1], 0);
        graph_add_edge_v(graph, &vertices[1], &vertices[2], 0);
        graph_add_edge_v(graph, &vertices[2], &vertices[0], 0);
        return;
    }

    int newsize = size / 2;
    int isodd = size & 1;

    delaunay_triangulation_f(vertices, newsize, graph);
    delaunay_triangulation_f(&vertices[newsize], isodd ? newsize + 1 : newsize, graph);

    //get base edge

    int i;
    struct vertex *lowl = NULL, *lowr = NULL;
    struct edge *inter = NULL;
    do
    {
        if(lowl && lowr)
        {
            if(graph_get_vertex(graph, inter->to) < &vertices[newsize] && graph_get_vertex(graph, inter->from) < &vertices[newsize])
                lowl = NULL;
            else
                lowr = NULL;
        }

        if(!lowl)
            for(i = 0; i < newsize; i++)
                if(!vertices[i].mark && (!lowl || get_y(lowl->p) >= get_y(vertices[i].p)))
                    lowl = &vertices[i];

        if(!lowr)
            for(i = newsize; i < size; i++)
                if(!vertices[i].mark && (!lowr || get_y(lowr->p) > get_y(vertices[i].p)))
                    lowr = &vertices[i];

        lowl->mark = 1;
        lowr->mark = 1;
    } while(intersect(graph, lowl, lowr, &inter));

    graph_add_edge_v(graph, lowl, lowr, 0);

    for(i = 0; i < size; i++)
        vertices[i].mark = 0;

    //get candidates, loop

    struct vertex *rcandidate = get_candidate(graph, lowr, lowl, 0), *lcandidate = get_candidate(graph, lowl, lowr, 1);
    struct vertex *rbase = lowr, *lbase = lowl;
    while(rcandidate || lcandidate)
    {
        if(!rcandidate)
        {
            graph_add_edge_v(graph, lcandidate, rbase, 0);
            lbase = lcandidate;
        }
        else if(!lcandidate)
        {
            graph_add_edge_v(graph, rcandidate, lbase, 0);
            rbase = rcandidate;
        }
        else
        {
            if(inside_circumcircle(lcandidate->p, lbase->p, rbase->p, rcandidate->p))
            {
                graph_add_edge_v(graph, lbase, rcandidate, 0);
                rbase = rcandidate;
            }
            else
            {
                graph_add_edge_v(graph, rbase, lcandidate, 0);
                lbase = lcandidate;
            }
        }

        rcandidate = get_candidate(graph, rbase, lbase, 0);
        lcandidate = get_candidate(graph, lbase, rbase, 1);
    }
    //both candidates null
}

static int get_angle(struct vertex *one, struct vertex *two, struct vertex *three)
{
    if(!one || !two || !three)
        return 182;

    float d12, d23, d13, x, y, ax, ay;
    x = get_x(one->p) - get_x(two->p);
    y = get_y(one->p) - get_y(two->p);
    ax = x;
    ay = y;
    d12 = math_sqrt(x * x + y * y);
    x = get_x(two->p) - get_x(three->p);
    y = get_y(two->p) - get_y(three->p);
    if(ax * y - ay * x < 0)//cross product
        return 181;
    d23 = math_sqrt(x * x + y * y);
    x = get_x(one->p) - get_x(three->p);
    y = get_y(one->p) - get_y(three->p);
    d13 = math_sqrt(x * x + y * y);
    x = (d12 * d12 + d23 * d23 - d13 * d13) / (2 * d12 * d23);
    return math_arccos_d(x);
}

static struct vertex *get_candidate(struct graph *graph, struct vertex *target, struct vertex *neighbor, int ccw)
{
    struct vertex *out = NULL, *next = NULL;
    int i, j, outedge, *order, *angles;
    order = s_malloc((target->noedges + 1) * sizeof(int), NULL);
    angles = s_malloc((target->noedges + 1) * sizeof(int), NULL);

    if(ccw)
        for(i = 0; i < target->noedges; i++)
            angles[i] = get_angle(graph_get_next_vertex(graph, target, i), target, neighbor);
    else
        for(i = 0; i < target->noedges; i++)
            angles[i] = get_angle(neighbor, target, graph_get_next_vertex(graph, target, i));
    
    /*for(i = 0; i < target->noedges; i++)
        printf("(%d)", angles[i]);
    puts("");*/
    
    for(i = 0; i < target->noedges; i++)
    {
        order[i] = i;

        for(j = 0; j < target->noedges; j++)
            if(angles[order[i]] > angles[j])
                order[i] = j;

        angles[order[i]] += 183 + i;
    }

    for(i = 0; i < target->noedges; i++)
    {
        angles[order[i]] -= 183 + i;
        //printf("(%d)", order[i]);
    }
    //puts("");
    
    angles[target->noedges] = -1;
    order[target->noedges] = target->noedges;

    //printf("%d\n", target->noedges);

    /*for(i = 0; i < target->noedges; i++)
        printf("%d ", order[i]);
    puts("");*/

    for(i = 0; i < target->noedges; i++)
    {
        out = graph_get_next_vertex(graph, target, order[i]);
        if(!out)
            continue;
        if(out == neighbor || angles[order[i]] >= 180 || angles[order[i]] <= 1)
        {
            out = NULL;
            continue;
        }

        next = graph_get_next_vertex(graph, target, order[i + 1]);
        
        if(next == neighbor)
            continue;

        if(angles[order[i + 1]] >= 180)
            break;
        if(!next)
            break;

        if(ccw)
        {
            if(inside_circumcircle(out->p, target->p, neighbor->p, next->p))//inside_circumcircle(out->p, target->p, neighbor->p, next->p)
            {
                //printf("ccw removed edge %d (%d, %d), (%d, %d), (%d, %d), (%d, %d)\n", i, get_x(out->p), get_y(out->p), get_x(target->p), get_y(target->p), get_x(neighbor->p), get_y(neighbor->p), get_x(next->p), get_y(next->p));
                graph_remove_edge(graph, graph_get_edge(graph, target, order[i]));
                out = NULL;
                continue;
            }
        }
        else
        {
            if(inside_circumcircle(neighbor->p, target->p, out->p, next->p))//inside_circumcircle(neighbor->p, target->p, out->p, next->p)
            {
                //printf("!ccw removed edge %d (%d, %d), (%d, %d), (%d, %d), (%d, %d)\n", i, get_x(neighbor->p), get_y(neighbor->p), get_x(target->p), get_y(target->p), get_x(out->p), get_y(out->p), get_x(next->p), get_y(next->p));
                graph_remove_edge(graph, graph_get_edge(graph, target, order[i]));
                out = NULL;
                continue;
            }
        }
        break;
    }

    s_free(order, NULL);
    s_free(angles, NULL);

    /*if(out)
        printf("out edge: %d (%d, %d), (%d, %d), (%d, %d)\n", i, get_x(out->p), get_y(out->p), get_x(target->p), get_y(target->p), get_x(neighbor->p), get_y(neighbor->p));
    else
        printf("out edge null: (%d, %d), (%d, %d)\n", get_x(target->p), get_y(target->p), get_x(neighbor->p), get_y(neighbor->p));
    */
    return out;
}

static int intersect(struct graph *graph, struct vertex *one, struct vertex *two, struct edge **inter)
{
    void *rone = one->p, *rtwo = two->p, *pone = NULL, *ptwo = NULL;
    struct vertex *vpone = NULL, *vptwo = NULL;
    *inter = NULL;
    int i;
    float m1 = (float)(get_y(rone) - get_y(rtwo)) / (float)(get_x(rone) - get_x(rtwo));
    float b1 = (float)get_y(rone) - (m1 * get_x(rone));
    for(i = 0; i < graph->noedges; i += 2)
    {
        vpone = graph_get_vertex(graph, graph->edges[i].to);
        vptwo = graph_get_vertex(graph, graph->edges[i].from);
        if(vpone && vptwo)
        {
            pone = vpone->p;
            ptwo = vptwo->p;

            float m2 = (float)(get_y(pone) - get_y(ptwo)) / (float)(get_x(pone) - get_x(ptwo));
            if(m1 == m2)
                continue;
            float b2 = (float)get_y(pone) - (m2 * get_x(pone));
            float x = (b2 - b1) / (m1 - m2);
            if((math_in_range(get_x(rone) + 1, x, get_x(rtwo) - 1) || math_in_range(get_x(rtwo) + 1, x, get_x(rone) - 1)) && (math_in_range(get_x(pone) + 1, x, get_x(ptwo) - 1) || math_in_range(get_x(ptwo) + 1, x, get_x(pone) - 1)))
            {
                *inter = &graph->edges[i];
                return 1;
            }
        }
    }
    return 0;
}

void delaunay_triangulation(struct graph *graph, int (*get_x_func)(void *), int (*get_y_func)(void *))
{
	get_x = get_x_func;
	get_y = get_y_func;

    if(graph->novertices > 1)
        delaunay_triangulation_f(graph->vertices, graph->novertices, graph);
}

static int inside_circumcircle(void *one, void *two, void *three, void *four)//one two and three must be in ccw order
{
    int x2 = get_x(four);
    x2 *= x2;
    int y2 = get_y(four);
    y2 *= y2;
    int a = get_x(one) - get_x(four);
    int b = get_y(one) - get_y(four);
    int c = (get_x(one) * get_x(one) - x2) + (get_y(one) * get_y(one) - y2);
    int d = get_x(two) - get_x(four);
    int e = get_y(two) - get_y(four);
    int f = (get_x(two) * get_x(two) - x2) + (get_y(two) * get_y(two) - y2);
    int g = get_x(three) - get_x(four);
    int h = get_y(three) - get_y(four);
    int i = (get_x(three) * get_x(three) - x2) + (get_y(three) * get_y(three) - y2);
    int orientation = a * e * i + b * f * g + c * d * h - c * e * g - f * h * a - i * b * d;
    return orientation > 0;
}