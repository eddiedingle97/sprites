#include <stdlib.h>
#include <stdio.h>
#include <allegro5/allegro.h>
#include "../entity.h"
#include "../keyboard.h"
#include "../sprites.h"
#include "../spritemanager.h"
#include "../map.h"
#include "../mapmanager.h"
#include "../emath.h"
#include "../util.h"
#include "../action.h"
#include "../graph.h"
#include "../entitymanager.h"
#include "entities.h"

enum NECROSTATE {IDLE, ESCAPE, SPAWN, AGGRO};

static struct animation *necroanimations = NULL;
static ALLEGRO_CONFIG *necrocfg = NULL;
static int nonecros = 0;
static int visiondist = 0;

struct astardata
{
    struct entity *necro;
    struct entity *target;
    float xcomp;
    float ycomp;
};

float a_star_cost(struct map *map, struct coord *cur, struct coord *end, void *data)
{
    struct astardata *d = data;
    float xcomp = math_get_vec_comp(d->necro->sprite->x, cur->x * map->tilesize);
    float ycomp = math_get_vec_comp(d->necro->sprite->y, cur->y * map->tilesize);

    if(!math_vec_norm(&xcomp, &ycomp));//do something here, or don't, shouldn't be catastrophic

    float dp = xcomp * d->xcomp + ycomp * d->ycomp;
    dp *= -1;
    dp += 1;//invert, then shift to value between 0 - 2, will cause vectors that "agree" to give smaller costs

    return (math_abs(cur->x - end->x) + math_abs(cur->y - end->y)) * dp;
}

struct room *pick_room(struct map *map, struct entity *e, struct entity *target)
{
    float chasex, chasey; //vector between target and e (should be target -> e)
    float escapex, escapey; //vector between e and proposed room (should be e -> escape)

    chasex = e->sprite->x - target->sprite->x;
    chasey = e->sprite->y - target->sprite->y;
    
    int i, farthestroom = -1;
    int curroom = map_get_room_index(map, e);
    float maxdp = 0.0f;
    for(i = 0; i < map->graph->novertices; i++)
    {
        escapex = (float)mg_room_center_x(&map->rooms[i]) * map->tilesize - e->sprite->x;
        escapey = (float)mg_room_center_y(&map->rooms[i]) * map->tilesize - e->sprite->y;
        float dp = escapex * chasex + escapey * chasey;
        if(dp > maxdp && graph_min_hops(map->graph, &map->graph->vertices[curroom], &map->graph->vertices[i]) >= 0)
        {
            maxdp = dp;
            farthestroom = i;
        }
    }

    if(farthestroom == -1)
        return NULL;
    return &map->rooms[farthestroom];
}

void necro_behaviour(struct map *map, struct entity *entity, float *dx, float *dy)
{
    struct sprite *sprite = entity->sprite;
    struct necrodata *data = entity->data;
    struct sprite *target = data->target->sprite;
    struct graph *graph = map->graph;
    float dist = eu_lerp_check(map, sprite->x, sprite->y, target->x, target->y, SOLID);
    float attackdist = entity->colrad * 20;//FIX: ought to be done in config
    int canescape = 1;//if some threshold of some cost function is met, set to 1
    //printf("%.2f %.2f %.2f\n", dist, entity->sprite->x, entity->sprite->y);
    if(entity->health > 0)
    {
        switch(data->state)//state transitions
        {
            case IDLE:
                if(dist != 0 && dist < attackdist)
                    data->state = AGGRO;
                if(dist != 0 && dist < visiondist)
                    data->state = ESCAPE;
                break;
            case ESCAPE:
                if(data->nexttile == data->escaperoutesize)
                    data->state = SPAWN;
                if(dist != 0 && (dist < attackdist || !data->escapeplan))
                    data->state = AGGRO;
                break;
            case SPAWN:
                if(dist != 0 && dist < attackdist)
                    data->state = AGGRO;
                if(dist != 0 && dist < visiondist)
                    data->state = ESCAPE;
                break;
            case AGGRO:
                if(dist == 0 || (dist > attackdist) && canescape)
                    data->state = ESCAPE;
                break;
        }

        switch(data->state)//state actions
        {
            case IDLE:
                break;
            case ESCAPE:
                //puts("escape");
                if(!data->escapeplan)
                {
                    data->escapeplan = pick_room(map, entity, data->target);
                    if(!data->escapeplan)
                    {
                        //puts("no escape");
                        break;
                    }
                       
                    //use a function pointer instead of tile mask in the future for better a star paths (i.e. takes into account other entities...)
                    if(!data->escaperoute)
                    {
                        struct astardata *adata = s_malloc(sizeof(struct astardata), NULL);
                        adata->necro = entity;
                        adata->target = data->target;
                        adata->xcomp = math_get_vec_comp(data->target->sprite->x, entity->sprite->x);
                        adata->ycomp = math_get_vec_comp(data->target->sprite->y, entity->sprite->y);

                        if(!math_vec_norm(&adata->xcomp, &adata->ycomp))//should never happen, will be in aggro next frame either way
                        {
                            s_free(adata, NULL);
                            data->escapeplan = NULL;
                            break;
                        }

                        data->escaperoute = eu_a_star(map, entity->sprite->x, entity->sprite->y, 
                        mg_room_center_x(data->escapeplan) * map->tilesize, mg_room_center_y(data->escapeplan) * map->tilesize, &data->escaperoutesize, SOLID,
                        a_star_cost, adata);
                        s_free(adata, NULL);
                        data->nexttile = 1;
                    }
                }
                else
                {
                    if(!data->escaperoute)//if no route is found
                    {
                        data->escapeplan = NULL;
                        break;
                    }
                    //define some way to exit this state and enter spawn state, preferably before it reaches the proposed room (i.e. data->escapeplan)
                    int curroom = map_get_room_index(map, entity);
                    int enemyroom = map_get_room_index(map, data->target);

                    if(eu_follow_path(map, entity, data->escaperoute, data->escaperoutesize, &data->nexttile, dx, dy))
                    {
                        s_free(data->escaperoute, NULL);
                        data->escaperoute = NULL;
                        data->escapeplan = NULL;
                    }
                }
                break;
            case SPAWN:
                //FIX: probably create a new action to do this
                if(data->cooldown == 0 && data->nospawned < data->maxspawned)
                {
                    struct entity *neworc = em_create_entity(1, entity->sprite->x + (1 + math_get_random(2)) * map->tilesize, entity->sprite->y + (1 + math_get_random(2)) * map->tilesize);
                    struct orcdata *od = neworc->data;
                    od->target = data->target;
                    em_add_entity_to_map(map, neworc);
                    data->cooldown = 120;
                    data->nospawned++;
                }
                else if(data->cooldown != 0)
                {
                    data->cooldown--;
                }
                break;
            case AGGRO:
                if(data->cooldown == 0)
                {
                    struct entity *pot = em_create_entity(6, entity->sprite->x, entity->sprite->y);
                    em_add_entity_to_map(map, pot);
                    action_throw(entity, pot, data->target->sprite->x - entity->sprite->x, data->target->sprite->y - entity->sprite->y, 2);
                    data->cooldown = 240;//use different variable for cooldown
                }
                else
                    data->cooldown--;

                /**dx = entity->sprite->x - data->target->sprite->x;
                *dy = entity->sprite->y - data->target->sprite->y;*/
                break;
        }
    }

    else
        data->state = IDLE;

    if(*dx < 0)
        sprite->alflags |= ALLEGRO_FLIP_HORIZONTAL;

    else if(*dx > 0)
        sprite->alflags = 0;

    sprite->i = 0;//data->state == IDLE;
}

struct entity *necro_create()
{
    struct necrodata *nd = s_malloc(sizeof(struct necrodata), NULL);
    nd->nexttile = 0;
    nd->escaperoutesize = 0;
    nd->state = IDLE;
    nd->cooldown = 0;
    nd->escapeplan = NULL;
    nd->escaperoute = NULL;
    nd->maxspawned = 5;
    nd->nospawned = 0;
    struct animation *an;
    if(!necrocfg)
    {
        necrocfg = al_load_config_file(s_get_full_path_with_dir("config/entities", "necro.cfg"));
        an = e_load_animations_from_config(necrocfg);
        visiondist = u_atoi(al_get_config_value(necrocfg, "stats", "visiondistance")) * 16;

        necroanimations = an;
    }
    else
        an = necroanimations;

    struct entity *out = e_create(0, 0, an, nd);
    e_load_stats_from_config(necrocfg, out);
    nonecros++;
    return out;
}

void necro_destroy(struct entity *necro)
{
    if(--nonecros == 0)
    {
        s_free(necroanimations, NULL);
        necroanimations = NULL;
        al_destroy_config(necrocfg);
        necrocfg = NULL;
    }
    struct necrodata *nd = necro->data;
    if(nd->escaperoute)
        s_free(nd->escaperoute, NULL);

    e_destroy(necro);
}