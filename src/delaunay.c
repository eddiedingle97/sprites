#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro.h>
#include "sprites.h"
#include "spritemanager.h"
#include "emath.h"
#include "graph.h"
#include "delaunay.h"
#include "debug.h"

static int (*get_x)(void *);
static int (*get_y)(void *);
static void delaunay_triangulation_f(struct vertex *vertices, int size, struct graph *graph);
static int inside_circumcircle(void *one, void *two, void *three, void *four);
static float get_angle(struct vertex *one, struct vertex *two, struct vertex *three);
struct vertex *get_next_vertex_from_vertical(struct graph *graph, struct vertex *v, int ccw);
static int counter_clockwise(struct vertex *one, struct vertex *two, struct vertex *three);
static struct vertex *get_candidate(struct graph *graph, struct vertex *target, struct vertex *neighbor, int ccw);
static int intersect(struct graph *graph, struct vertex *one, struct vertex *two, struct edge **inter);

static void delaunay_triangulation_f(struct vertex *vertices, int size, struct graph *graph)
{
    if(size == 2)
    {
        graph_add_edge_v(graph, &vertices[0], &vertices[1], math_get_distance(get_x(vertices[0].p) - get_x(vertices[1].p), get_y(vertices[0].p) - get_y(&vertices[1].p)));
        return;
    }

    if(size == 3)
    {
        graph_add_edge_v(graph, &vertices[0], &vertices[1], math_get_distance(get_x(vertices[0].p) - get_x(vertices[1].p), get_y(vertices[0].p) - get_y(vertices[1].p)));
        graph_add_edge_v(graph, &vertices[1], &vertices[2], math_get_distance(get_x(vertices[1].p) - get_x(vertices[2].p), get_y(vertices[1].p) - get_y(vertices[2].p)));
        graph_add_edge_v(graph, &vertices[2], &vertices[0], math_get_distance(get_x(vertices[2].p) - get_x(vertices[0].p), get_y(vertices[2].p) - get_y(vertices[0].p)));
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
            if(graph_get_vertex(graph, inter->to) < &vertices[newsize] && graph_get_vertex(graph, inter->from) < &vertices[newsize])//if intercepting edge was in left set
                lowl = NULL;
            else//otherwise intercepting edge SHOULD be in right set
                lowr = NULL;
            if(debug_get())
            {
                if(graph_get_vertex(graph, inter->to) < &vertices[newsize] && graph_get_vertex(graph, inter->from) >= &vertices[newsize])
                    debug_printf("intercepting edge in delaunay triangulation was between left and right subset\n");
                else if(graph_get_vertex(graph, inter->to) >= &vertices[newsize] && graph_get_vertex(graph, inter->from) < &vertices[newsize])
                    debug_printf("intercepting edge in delaunay triangulation was between left and right subset\n");
            }
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

    struct vertex *lowlnextv = get_next_vertex_from_vertical(graph, lowl, 0), *lowrnextv = get_next_vertex_from_vertical(graph, lowr, 1);

    while(1)
    {
        int lccw = counter_clockwise(lowr, lowl, lowlnextv);
        int rccw = counter_clockwise(lowl, lowrnextv, lowr);
        if(lccw > 0)
        {
            lowl = lowlnextv;
            lowlnextv = get_next_vertex_from_vertical(graph, lowlnextv, 0);
        }
        else if(rccw > 0)
        {
            lowr = lowrnextv;
            lowrnextv = get_next_vertex_from_vertical(graph, lowrnextv, 1);
        }
        else
            break;
    }

    graph_add_edge_v(graph, lowl, lowr, math_get_distance(get_x(lowl->p) - get_x(lowr->p), get_y(lowl->p) - get_y(lowr->p)));

    for(i = 0; i < size; i++)
        vertices[i].mark = 0;

    //get candidates, loop

    struct vertex *rcandidate = get_candidate(graph, lowr, lowl, 0), *lcandidate = get_candidate(graph, lowl, lowr, 1);
    struct vertex *rbase = lowr, *lbase = lowl;
    while(rcandidate || lcandidate)
    {
        if(!rcandidate)
        {
            graph_add_edge_v(graph, lcandidate, rbase, math_get_distance(get_x(lcandidate->p) - get_x(rbase->p), get_y(lcandidate->p) - get_y(rbase->p)));
            lbase = lcandidate;
        }
        else if(!lcandidate)
        {
            graph_add_edge_v(graph, rcandidate, lbase, math_get_distance(get_x(rcandidate->p) - get_x(lbase->p), get_y(rcandidate->p) - get_y(lbase->p)));
            rbase = rcandidate;
        }
        else
        {
            if(inside_circumcircle(lcandidate->p, lbase->p, rbase->p, rcandidate->p))
            {
                graph_add_edge_v(graph, lbase, rcandidate, math_get_distance(get_x(rcandidate->p) - get_x(lbase->p), get_y(rcandidate->p) - get_y(lbase->p)));
                rbase = rcandidate;
            }
            else
            {
                graph_add_edge_v(graph, rbase, lcandidate, math_get_distance(get_x(lcandidate->p) - get_x(rbase->p), get_y(lcandidate->p) - get_y(rbase->p)));
                lbase = lcandidate;
            }
        }

        rcandidate = get_candidate(graph, rbase, lbase, 0);
        lcandidate = get_candidate(graph, lbase, rbase, 1);
    }
    //both candidates null
}

static float get_angle(struct vertex *one, struct vertex *two, struct vertex *three)
{
    if(!one || !two || !three)
        return -1.0f;

    float d12, d23, d13;
    long x = 0, y = 0, ax = 0, ay = 0;
    x = get_x(one->p) - get_x(two->p);
    y = get_y(one->p) - get_y(two->p);
    ax = x;
    ay = y;
    d12 = math_sqrt(x * x + y * y);
    x = get_x(two->p) - get_x(three->p);
    y = get_y(two->p) - get_y(three->p);
    if(ax * y - ay * x <= 0)//cross product, use ints for precision. I realize now this is weird for two vectors of different origin... maybe change this in the future, it works for now
        return -1.0f;//if < 0 discard, don't need negative angles for candidate function, if 0 discard, angle is either 0 or 180 degrees. FIX: maybe don't discard angles of 0?
    d23 = math_sqrt(x * x + y * y);
    x = get_x(one->p) - get_x(three->p);
    y = get_y(one->p) - get_y(three->p);
    d13 = math_sqrt(x * x + y * y);
    float result = (d12 * d12 + d23 * d23 - d13 * d13) / (2 * d12 * d23);
    if(result < -1.0f)
        return 180.0f;
    else if(result > 1.0f)
        return 0.0f;
    else
        return math_arccos(result) * 180.0f / M_PI;
}

float *angles = NULL;
static int candidate_compare(int *one, int *two)
{
    if(!angles)
        return 0;
    if(angles[*one] - angles[*two] > 0)
        return 1;
    if(angles[*one] - angles[*two] < 0)
        return -1;
    return 0;
}

static struct vertex *get_candidate(struct graph *graph, struct vertex *target, struct vertex *neighbor, int ccw)
{
    struct vertex *out = NULL, *next = NULL;
    int i, j, outedge, *order, *taken;
    order = s_malloc((target->noedges + 1) * sizeof(int), NULL);
    angles = s_malloc((target->noedges + 1) * sizeof(float), NULL);

    if(ccw)
        for(i = 0; i < target->noedges; i++)
            angles[i] = get_angle(graph_get_next_vertex(graph, target, i), target, neighbor);
    else
        for(i = 0; i < target->noedges; i++)
            angles[i] = get_angle(neighbor, target, graph_get_next_vertex(graph, target, i));
            

    for(i = 0; i < target->noedges + 1; i++)
        order[i] = i;


    math_mergesort(order, target->noedges, candidate_compare, sizeof(int));
    
    angles[target->noedges] = -1;
    order[target->noedges] = target->noedges;

    for(i = 0; i < target->noedges; i++)
    {
        out = graph_get_next_vertex(graph, target, order[i]);
        if(!out)
            continue;
        if(out == neighbor || angles[order[i]] >= 180 || angles[order[i]] < 0)
        {
            out = NULL;
            continue;
        }

        next = graph_get_next_vertex(graph, target, order[i + 1]);

        if(next == neighbor)
            continue;

        if(angles[order[i + 1]] >= 180 || angles[order[i + 1]] == -1)
            break;
        if(!next)
            break;

        if(ccw)
        {
            if(inside_circumcircle(out->p, target->p, neighbor->p, next->p))//inside_circumcircle(out->p, target->p, neighbor->p, next->p)
            {
                graph_remove_edge(graph, graph_get_edge(graph, target, order[i]));
                out = NULL;
                continue;
            }
        }
        else
        {
            if(inside_circumcircle(neighbor->p, target->p, out->p, next->p))//inside_circumcircle(neighbor->p, target->p, out->p, next->p)
            {
                graph_remove_edge(graph, graph_get_edge(graph, target, order[i]));
                out = NULL;
                continue;
            }
        }
        break;
    }

    s_free(order, NULL);
    s_free(angles, NULL);

    return out;
}

static int intersect(struct graph *graph, struct vertex *one, struct vertex *two, struct edge **inter)
{
    //puts("in intersect");
    int x1 = get_x(one->p), y1 = get_y(one->p), x2 = get_x(two->p), y2 = get_y(two->p);
    int x3, y3, x4, y4;
    double sx1, sy1, sx2, sy2, s, t;
    int i;
    *inter = NULL;
    sx1 = x2 - x1;
    sy1 = y2 - y1;
    for(i = 0; i < graph->noedges; i += 2)
    {
        struct edge *e = &graph->edges[i];
        struct vertex *three = graph_get_vertex(graph, e->from);//&graph->vertices[e->from];
        struct vertex *four = graph_get_vertex(graph, e->to);//&graph->vertices[e->to];
        
        if(three && four)
        {
            x3 = get_x(three->p);
            y3 = get_y(three->p);
            x4 = get_x(four->p);
            y4 = get_y(four->p);
            sx2 = x4 - x3;
            sy2 = y4 - y3;
            double denom = -sx2 * sy1 + sx1 * sy2;
            if(denom == 0)
                continue;
            s = (-sy1 * (x1 - x3) + sx1 * (y1 - y3));
            if((s < 0) == (denom > 0))
                continue;
            t = (-sy2 * (x1 - x3) + sx2 * (y1 - y3));
            if(s == 0 || t == 0 || s == denom || t == denom)
                continue;
            if((t < 0) == (denom > 0))
                continue;
            if(((t > denom) == (denom > 0)) || ((s > denom) == (denom > 0)))
                continue;
            
            //printf("exiting intersect found %p %d %d %ld %ld %.2f %.2f %.2f\n", e, e->to, e->from, one - graph->vertices, two - graph->vertices, s, t, denom);
            *inter = e;
            return 1;
        }
    }
    //puts("exiting intersect");
    return 0;
}

void delaunay_triangulation(struct graph *graph, int (*get_x_func)(void *), int (*get_y_func)(void *))
{
	get_x = get_x_func;
	get_y = get_y_func;

    if(debug_get())
    {
        int i;
        FILE *pointfile = fopen("points.csv", "w");
        for(i = 0; i < graph->novertices; i++)
            fprintf(pointfile, "%d, %d\n", get_x(graph->vertices[i].p), get_y(graph->vertices[i].p));
        fclose(pointfile);
    }

    if(graph->novertices > 1)
        delaunay_triangulation_f(graph->vertices, graph->novertices, graph);
    
    graph_unmark(graph);
}

ALLEGRO_MUTEX *mutex;
ALLEGRO_COND *cond;
static struct vertex *get_candidate_debug(struct graph *graph, struct vertex *target, struct vertex *neighbor, int ccw);
static void delaunay_triangulation_f_debug(struct vertex *vertices, int size, struct graph *graph);

void delaunay_triangulation_debug(ALLEGRO_THREAD *thread, struct delaunaydata *data)
{
    get_x = data->get_x;
    get_y = data->get_y;
    mutex = data->mutex;
    cond = data->cond;
    printf("%p %p %p %p\n", mutex, cond, get_x, get_y);

    delaunay_triangulation_f_debug(data->graph->vertices, data->graph->novertices, data->graph);
    if(graph_is_connected(data->graph))
    {
        puts("graph is connected");
    }
    else
        puts("graph is not connected");
    puts("done");
}

struct sprite *line = NULL;

/*
    x = get_x(one->p) - get_x(two->p);
    y = get_y(one->p) - get_y(two->p);
    ax = x;
    ay = y;
    d12 = math_sqrt(x * x + y * y);
    x = get_x(two->p) - get_x(three->p);
    y = get_y(two->p) - get_y(three->p);
    if(ax * y - ay * x <= 0)//cross product
        return -1.0f;
*/

struct vertex *get_next_vertex_from_vertical(struct graph *graph, struct vertex *v, int ccw)
{
    int i, smallestanglei = -1;
    struct vertex *w = NULL;
    
    float d12, d23, d13, vx, vy, angle, smallestangle = 4;
    vx = get_x(v->p);
    vy = get_y(v->p);
    d12 = 1;

    //printf("gnvfv %ld\n", v - graph->vertices);

    for(i = 0; i < v->noedges; i++)
    {
        w = graph_get_next_vertex(graph, v, i);
        if(w)
        {
            d13 = math_get_distance(vx - get_x(w->p), vy - get_y(w->p));
            d23 = math_get_distance(vx - get_x(w->p), vy + 1 - get_y(w->p));
            float inter = (d12 * d12 + d23 * d23 - d13 * d13) / (2 * d12 * d23);
            if(inter < -1.0f)
                angle = M_PI;
            else if(inter > 1.0f)
                angle = 0.0f;
            else
                angle = math_arccos(inter);

            if(ccw && get_x(w->p) > vx)
                angle += (M_PI - angle);
            else if(!ccw && get_x(w->p) < vx)
                angle += (M_PI - angle);

            if(angle < smallestangle)
            {
                smallestangle = angle;
                smallestanglei = i;
            }

            //printf("%.2f %d : %.2f %d %d %.2f %d, %.2f, %.2f, %.2f, %.10f\n", angle, i, smallestangle, smallestanglei, get_x(w->p), vx, get_x(w->p) < vx, d12, d23, d13,(d12 * d12 + d23 * d23 - d13 * d13) / (2 * d12 * d23));
        }
    }
    return graph_get_next_vertex(graph, v, smallestanglei);
}

static int counter_clockwise(struct vertex *one, struct vertex *two, struct vertex *three)
{
    //printf("in ccw function %p %p %p %d %d %d %d %d %d %d\n", one, two, three, ((get_x(two->p) - get_x(one->p)) * (get_y(three->p) - get_y(one->p)) - (get_y(two->p) - get_y(one->p)) * (get_x(three->p) - get_x(one->p))), (get_x(two->p) - get_x(one->p)) * (get_y(three->p) - get_y(one->p)), (get_y(two->p) - get_y(one->p)) * (get_x(three->p) - get_x(one->p)), get_x(two->p), get_x(one->p), get_y(three->p), get_y(one->p));
    return ((get_x(two->p) - get_x(one->p)) * (get_y(three->p) - get_y(one->p)) - (get_y(two->p) - get_y(one->p)) * (get_x(three->p) - get_x(one->p)));
}

static void delaunay_triangulation_f_debug(struct vertex *vertices, int size, struct graph *graph)
{
    if(size == 2)
    {
        al_lock_mutex(mutex);
        al_wait_cond(cond, mutex);
        al_unlock_mutex(mutex);
        line = sm_draw_line(get_x(vertices[0].p) * 16.0f, get_y(vertices[0].p) * 16.0f, get_x(vertices[1].p) * 16.0f, get_y(vertices[1].p) * 16.0f);
        graph_add_edge_vp(graph, &vertices[0], &vertices[1], math_get_distance(get_x(vertices[0].p) - get_x(vertices[1].p), get_y(vertices[0].p) - get_y(vertices[1].p)), line);
        return;
    }

    if(size == 3)
    {
        al_lock_mutex(mutex);
        al_wait_cond(cond, mutex);
        al_unlock_mutex(mutex);
        line = sm_draw_line(get_x(vertices[0].p) * 16.0f, get_y(vertices[0].p) * 16.0f, get_x(vertices[1].p) * 16.0f, get_y(vertices[1].p) * 16.0f);
        graph_add_edge_vp(graph, &vertices[0], &vertices[1], math_get_distance(get_x(vertices[0].p) - get_x(vertices[1].p), get_y(vertices[0].p) - get_y(vertices[1].p)), line);
        line = sm_draw_line(get_x(vertices[1].p) * 16.0f, get_y(vertices[1].p) * 16.0f, get_x(vertices[2].p) * 16.0f, get_y(vertices[2].p) * 16.0f);
        graph_add_edge_vp(graph, &vertices[1], &vertices[2], math_get_distance(get_x(vertices[1].p) - get_x(vertices[2].p), get_y(vertices[1].p) - get_y(vertices[2].p)), line);
        line = sm_draw_line(get_x(vertices[2].p) * 16.0f, get_y(vertices[2].p) * 16.0f, get_x(vertices[0].p) * 16.0f, get_y(vertices[0].p) * 16.0f);
        graph_add_edge_vp(graph, &vertices[2], &vertices[0], math_get_distance(get_x(vertices[2].p) - get_x(vertices[0].p), get_y(vertices[2].p) - get_y(vertices[0].p)), line);
        return;
    }

    int newsize = size / 2;
    int isodd = size & 1;

    delaunay_triangulation_f_debug(vertices, newsize, graph);
    delaunay_triangulation_f_debug(&vertices[newsize], isodd ? newsize + 1 : newsize, graph);

    //get base edge
    int i;
    struct vertex *lowl = NULL, *lowr = NULL;
    struct edge *inter = NULL;
    
    do
    {
        if(lowl && lowr)
        {
            if(graph_get_vertex(graph, inter->to) < &vertices[newsize] && graph_get_vertex(graph, inter->from) < &vertices[newsize])//if intercepting edge was in left set
                lowl = NULL;
            else//otherwise intercepting edge SHOULD be in right set
                lowr = NULL;
            if(debug_get())
            {
                if(graph_get_vertex(graph, inter->to) < &vertices[newsize] && graph_get_vertex(graph, inter->from) >= &vertices[newsize])
                    debug_printf("intercepting edge in delaunay triangulation was between left and right subset\n");
                else if(graph_get_vertex(graph, inter->to) >= &vertices[newsize] && graph_get_vertex(graph, inter->from) < &vertices[newsize])
                    debug_printf("intercepting edge in delaunay triangulation was between left and right subset\n");
            }
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

    //struct vertex *lowl = &vertices[newsize - 1], *lowr = &vertices[newsize];
    struct vertex *lowlnextv = get_next_vertex_from_vertical(graph, lowl, 0), *lowrnextv = get_next_vertex_from_vertical(graph, lowr, 1);
    printf("picking base edge %ld %ld %p\n", lowl - graph->vertices, lowr - graph->vertices, lowlnextv);

    while(1)
    {
        if(counter_clockwise(lowr, lowl, lowlnextv))
        {
            lowl = lowlnextv;
            lowlnextv = get_next_vertex_from_vertical(graph, lowlnextv, 0);
        }
        else if(counter_clockwise(lowl, lowrnextv, lowr))
        {
            lowr = lowrnextv;
            lowrnextv = get_next_vertex_from_vertical(graph, lowrnextv, 1);
        }
        else
            break;
    }

    printf("lowl %ld lowlnextv %ld lowr %ld lowrnextv %ld\n", lowl - graph->vertices, lowlnextv - graph->vertices, lowr - graph->vertices, lowrnextv - graph->vertices);
    al_lock_mutex(mutex);
    al_wait_cond(cond, mutex);
    al_unlock_mutex(mutex);
    printf("base edge, %ld %ld\n", lowl - graph->vertices, lowr - graph->vertices);
    line = sm_draw_line(get_x(lowl->p) * 16.0f, get_y(lowl->p) * 16.0f, get_x(lowr->p) * 16.0f, get_y(lowr->p) * 16.0f);
    graph_add_edge_vp(graph, lowl, lowr, math_get_distance(get_x(lowl->p) - get_x(lowr->p), get_y(lowl->p) - get_y(lowr->p)), line);

    for(i = 0; i < size; i++)
        vertices[i].mark = 0;

    //get candidates, loop

    struct vertex *rcandidate = get_candidate_debug(graph, lowr, lowl, 0), *lcandidate = get_candidate_debug(graph, lowl, lowr, 1);
    struct vertex *rbase = lowr, *lbase = lowl;
    while(rcandidate || lcandidate)
    {
        if(!rcandidate)
        {
            al_lock_mutex(mutex);
            al_wait_cond(cond, mutex);
            al_unlock_mutex(mutex);
            puts("rcandidate null");
            line = sm_draw_line(get_x(lcandidate->p) * 16.0f, get_y(lcandidate->p) * 16.0f, get_x(rbase->p) * 16.0f, get_y(rbase->p) * 16.0f);
            graph_add_edge_vp(graph, lcandidate, rbase, math_get_distance(get_x(lcandidate->p) - get_x(rbase->p), get_y(lcandidate->p) - get_y(rbase->p)), line);
            lbase = lcandidate;
        }
        else if(!lcandidate)
        {
            al_lock_mutex(mutex);
            al_wait_cond(cond, mutex);
            al_unlock_mutex(mutex);
            puts("lcandidate null");
            line = sm_draw_line(get_x(rcandidate->p) * 16.0f, get_y(rcandidate->p) * 16.0f, get_x(lbase->p) * 16.0f, get_y(lbase->p) * 16.0f);
            graph_add_edge_vp(graph, rcandidate, lbase, math_get_distance(get_x(rcandidate->p) - get_x(lbase->p), get_y(rcandidate->p) - get_y(lbase->p)), line);
            rbase = rcandidate;
        }
        else
        {
            if(inside_circumcircle(lcandidate->p, lbase->p, rbase->p, rcandidate->p))
            {
                al_lock_mutex(mutex);
                al_wait_cond(cond, mutex);
                al_unlock_mutex(mutex);
                printf("lcandidate lost: %ld, picking rcandidate: %ld\n", lcandidate - graph->vertices, rcandidate - graph->vertices);
                line = sm_draw_line(get_x(rcandidate->p) * 16.0f, get_y(rcandidate->p) * 16.0f, get_x(lbase->p) * 16.0f, get_y(lbase->p) * 16.0f);
                graph_add_edge_vp(graph, lbase, rcandidate, math_get_distance(get_x(rcandidate->p) - get_x(lbase->p), get_y(rcandidate->p) - get_y(lbase->p)), line);
                rbase = rcandidate;
            }
            else
            {
                al_lock_mutex(mutex);
                al_wait_cond(cond, mutex);
                al_unlock_mutex(mutex);
                printf("rcandidate lost: %ld, picking lcandidate: %ld\n", rcandidate - graph->vertices, lcandidate - graph->vertices);
                line = sm_draw_line(get_x(lcandidate->p) * 16.0f, get_y(lcandidate->p) * 16.0f, get_x(rbase->p) * 16.0f, get_y(rbase->p) * 16.0f);
                graph_add_edge_vp(graph, rbase, lcandidate, math_get_distance(get_x(lcandidate->p) - get_x(rbase->p), get_y(lcandidate->p) - get_y(rbase->p)), line);
                lbase = lcandidate;
            }
        }

        printf("lbase %ld, rbase %ld\n", lbase - graph->vertices, rbase - graph->vertices);
        rcandidate = get_candidate_debug(graph, rbase, lbase, 0);
        lcandidate = get_candidate_debug(graph, lbase, rbase, 1);
    }
    //both candidates null
}

static struct vertex *get_candidate_debug(struct graph *graph, struct vertex *target, struct vertex *neighbor, int ccw)
{
    struct vertex *out = NULL, *next = NULL;
    int i, j, outedge, *order;
    order = s_malloc((target->noedges + 1) * sizeof(int), NULL);
    angles = s_malloc((target->noedges + 1) * sizeof(float), NULL);

    if(ccw)
        for(i = 0; i < target->noedges; i++)
            angles[i] = get_angle(graph_get_next_vertex(graph, target, i), target, neighbor);
    else
        for(i = 0; i < target->noedges; i++)
            angles[i] = get_angle(neighbor, target, graph_get_next_vertex(graph, target, i));
            
    for(i = 0; i < target->noedges + 1; i++)
        order[i] = i;

    math_mergesort(order, target->noedges, candidate_compare, sizeof(int));
    
    angles[target->noedges] = -1;
    order[target->noedges] = target->noedges;

    for(i = 0; i < target->noedges; i++)
        printf("%.2f\n", angles[order[i]]);

    /*if(ccw)
        puts("left candidate");
    else
        puts("right candidate");

    for(i = 0; i < target->noedges; i++)
        printf("%.2f\n", angles[order[i]]);*/

    for(i = 0; i < target->noedges; i++)
    {
        out = graph_get_next_vertex(graph, target, order[i]);
        if(!out)
            continue;
        if(out == neighbor || angles[order[i]] >= 180 || angles[order[i]] < 0)
        {
            out = NULL;
            continue;
        }
        if(!ccw)
            printf("out vertex: %ld, angle: %.2f\n", out - graph->vertices, angles[order[i]]);

        next = graph_get_next_vertex(graph, target, order[i + 1]);

        if(next == neighbor)
            continue;

        if(angles[order[i + 1]] >= 180 || angles[order[i + 1]] == -1)
            break;
        if(!next)
            break;

        if(ccw)
        {
            if(inside_circumcircle(out->p, target->p, neighbor->p, next->p))
            {
                struct edge *e = graph_get_edge(graph, target, order[i]);
                al_lock_mutex(mutex);
                al_wait_cond(cond, mutex);
                al_unlock_mutex(mutex);
                printf("left candidate removing edge\n");
                graph_remove_edge(graph, e);
                sm_destroy_sprite(e->p);
                out = NULL;
                continue;
            }
        }
        else
        {
            if(inside_circumcircle(neighbor->p, target->p, out->p, next->p))
            {
                struct edge *e = graph_get_edge(graph, target, order[i]);
                al_lock_mutex(mutex);
                al_wait_cond(cond, mutex);
                al_unlock_mutex(mutex);
                printf("right candidate removing edge\n");
                graph_remove_edge(graph, e);
                sm_destroy_sprite(e->p);
                out = NULL;
                continue;
            }
        }
        
        break;
    }

    s_free(order, NULL);
    s_free(angles, NULL);

    return out;
}

static int inside_circumcircle(void *one, void *two, void *three, void *four)//one two and three must be in ccw order
{
    float x2 = get_x(four);
    x2 *= x2;
    float y2 = get_y(four);
    y2 *= y2;
    float a = get_x(one) - get_x(four);
    float b = get_y(one) - get_y(four);
    float c = (get_x(one) * get_x(one) - x2) + (get_y(one) * get_y(one) - y2);
    float d = get_x(two) - get_x(four);
    float e = get_y(two) - get_y(four);
    float f = (get_x(two) * get_x(two) - x2) + (get_y(two) * get_y(two) - y2);
    float g = get_x(three) - get_x(four);
    float h = get_y(three) - get_y(four);
    float i = (get_x(three) * get_x(three) - x2) + (get_y(three) * get_y(three) - y2);
    float orientation = a * e * i + b * f * g + c * d * h - c * e * g - f * h * a - i * b * d;
    if(debug_get())
    {
        double dorientation = (double)a * e * i + (double)b * f * g + (double)c * d * h - (double)c * e * g - (double)f * h * a - (double)i * b * d;
        if((dorientation > 0) ^ (orientation > 0))
            debug_printf("likely precision error in inside_circumcircle function in delaunay.c %.2f, %.2f\n", dorientation, orientation);
    }
    return orientation > 0;
}

/*int delaunay_correctness(struct graph *graph, int (*get_x_func)(void *), int (*get_y_func)(void *))
{
	get_x = get_x_func;
	get_y = get_y_func;
}*/
