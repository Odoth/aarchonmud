/* Countdown timers for progs */
/* Written by Vodur for Aarchon MUD
   Clayton Richey, clayton.richey@gmail.com
   */
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "timer.h"

#define GO_TYPE_UNDEFINED 0
#define GO_TYPE_CH 1
#define GO_TYPE_OBJ 2
#define GO_TYPE_AREA 3
#define GO_TYPE_ROOM 4

#define TM_UNDEFINED 0
#define TM_PROG      1
#define TM_LUAFUNC   2

/* hide the struct implementation,
   we only want to manipulate nodes
   in this module */
struct timer_node
{
    struct timer_node *next;
    struct timer_node *prev;
    int tm_type;
    void *game_obj;
    int go_type;
    int current; /* current val that gets decremented each second */
    bool unregistered; /* to mark for deletion */
    const char *tag; /* used for unique tags in lua */
};


TIMER_NODE *first_timer=NULL;

static void add_timer( TIMER_NODE *tmr);
static void remove_timer( TIMER_NODE *tmr );
static void free_timer_node( TIMER_NODE *tmr);
static TIMER_NODE *new_timer_node( void *gobj, int go_type, int tm_type, int max, const char *tag );

TIMER_NODE * register_lua_timer( int value, const char *tag)
{
    TIMER_NODE *tmr=new_timer_node( NULL , GO_TYPE_UNDEFINED, TM_LUAFUNC, value, tag );
    add_timer(tmr);
    
    return tmr;
}

/* unregister timer and return true if tag matches given tag, else return false*/
bool unregister_lua_timer( TIMER_NODE *tmr, const char *tag )
{
    if ( tag==NULL )
    {
        if ( tmr->tag != NULL )
        {
            return FALSE;
        }
        remove_timer(tmr);
        return TRUE;
    }
    else if (!strcmp(tag, "*"))
    {
        remove_timer(tmr);
        return TRUE;
    }
    else if ( !tmr->tag )
    {
        return FALSE;
    }
    else if ( !strcmp( tag, tmr->tag) )
    {
        remove_timer(tmr);
        return TRUE;
    }

    return FALSE;
}

/* register on the list and also return a pointer to the node
   in the form of void */
TIMER_NODE * register_ch_timer( CHAR_DATA *ch, int max )
{
    if (!valid_CH( ch ))
    {
        bugf("Trying to register timer for invalid CH");
        return NULL;
    }
    if (ch->must_extract)
    {
        bugf("Trying to register timer for CH pending extraction");
        return NULL;
    }
    if ( ch->trig_timer)
    {
        bugf("Tying to register timer for %s but already registered.", ch->name);
        return NULL;
    }

    TIMER_NODE *tmr=new_timer_node( (void *)ch, GO_TYPE_CH, TM_PROG, max, NULL);

    add_timer(tmr);

    ch->trig_timer=tmr;

    return tmr;

}

/* register on the list and also return a pointer to the node
   in the form of void */
TIMER_NODE * register_obj_timer( OBJ_DATA *obj, int max )
{
    if ( obj->otrig_timer)
    {
        bugf("Tying to register timer for %s but already registered.", obj->name);
        return NULL;
    }

    TIMER_NODE *tmr=new_timer_node( (void *)obj, GO_TYPE_OBJ, TM_PROG, max, NULL);

    add_timer(tmr);

    obj->otrig_timer=tmr;

    return tmr;

}

/* register on the list and also return a pointer to the node
   in the form of void */
TIMER_NODE * register_area_timer( AREA_DATA *area, int max )
{
    if ( area->atrig_timer)
    {
        bugf("Tying to register timer for %s but already registered.", area->name);
        return NULL;
    }

    TIMER_NODE *tmr=new_timer_node( (void *)area, GO_TYPE_AREA, TM_PROG, max, NULL);

    add_timer(tmr);

    area->atrig_timer=tmr;

    return tmr;

}

TIMER_NODE * register_room_timer( ROOM_INDEX_DATA *room, int max )
{
    if ( room->rtrig_timer)
    {
        bugf("Trying to register timer for room %d but already registered.", room->vnum);
        return NULL;
    }

    TIMER_NODE *tmr=new_timer_node( (void *)room, GO_TYPE_ROOM, TM_PROG, max, NULL);

    add_timer(tmr);

    room->rtrig_timer=tmr;

    return tmr;
}


static void add_timer( TIMER_NODE *tmr)
{
    if (first_timer)
        first_timer->prev=tmr;
    tmr->next=first_timer;
    first_timer=tmr;

}

static void remove_timer( TIMER_NODE *tmr )
{
    if ( tmr->prev)
        tmr->prev->next=tmr->next;
    if ( tmr->next)
        tmr->next->prev=tmr->prev;
    if ( tmr==first_timer )
        first_timer=tmr->next;

    free_timer_node(tmr);
    return;
}

void unregister_ch_timer( CHAR_DATA *ch )
{
    if (!ch->trig_timer)
    {
        /* doesn't have one */
        return;
    }
    TIMER_NODE *tmr=(TIMER_NODE *)ch->trig_timer;

    tmr->unregistered=TRUE; /* queue it for removal next update */ 
    ch->trig_timer=NULL;
    return;
}

void unregister_obj_timer( OBJ_DATA *obj )
{
    if (!obj->otrig_timer)
    {
        /* doesn't have one */
        return;
    }
    TIMER_NODE *tmr=obj->otrig_timer;

    tmr->unregistered=TRUE; /* queue it for removal next update */
    obj->otrig_timer=NULL;
    return;
}

static void free_timer_node( TIMER_NODE *tmr)
{
    free_string(tmr->tag);
    free_mem(tmr, sizeof(TIMER_NODE));
}

static TIMER_NODE *new_timer_node( void *gobj, int go_type, int tm_type, int seconds, const char *tag )
{
    TIMER_NODE *new=alloc_mem(sizeof(TIMER_NODE));
    new->next=NULL;
    new->prev=NULL;
    new->tm_type=tm_type;
    new->game_obj=gobj;
    new->go_type=go_type;
    new->current=seconds;
    new->unregistered=FALSE;
    new->tag=str_dup(tag);
    return new;
}

/* Should be called every second */
/* need to solve the problem of gobj destruction and unregistering
   screwing up loop iteration and crashing mud */
/* what happens when tmr_next is destroyed by current tmr */
/* condition 1
   tmr_next gobj is destroyed by prog of tmr
   tmr_next is unregistered and destroyed but we are still
   pointing to it
   how do we prevent this happening?
   set tmr-next AFTER the prog processes but before we destroy tmr

   but what if tmr gobj was destroyed during the prog?
   we could set tmr next at the top of the loop but also
   re-set it after the prog just in case

   this would work in cases where only tmr_next was destroyed
   or only tmr was destroyed, but what if both were destroyed?

   we probably need to unregister timers outside of progs
   ( similar to must_extract ) to avoid all of these shenanigans.
   simply set the must_extract bit. if we destroyed something
   we already looped past, we grab it on the next timer_update.
   if we destroyed something not looped yet, we just cleanly
   unregister/free it when its turn is up */

void timer_update()
{
    TIMER_NODE *tmr, *tmr_next;
    CHAR_DATA *ch;
    OBJ_DATA *obj;
    AREA_DATA *area;
    ROOM_INDEX_DATA *room;

    for (tmr=first_timer ; tmr ; tmr=tmr_next)
    {
        tmr_next=tmr->next;

        if ( tmr->unregistered )
        {
            /* it was unregistered since the last update
               we need to kill it cleanly */
            remove_timer( tmr );
            continue;
        }

        tmr->current-=1;
        if (tmr->current <= 0)
        {
            switch(tmr->tm_type)
            {
                case TM_PROG:
                    switch( tmr->go_type )
                    {
                        case GO_TYPE_CH: 
                            ch=(CHAR_DATA *)(tmr->game_obj);
                            if (ch->must_extract)
                                break;

                            if (!valid_CH( ch ) )
                            {
                                /* Shouldn't happen ever */
                                bugf("timer_update: invalid ch");
                                break;
                            }
                            mp_timer_trigger( ch );
                            /* repeating timer, set it up again */
                            if (valid_CH( ch ) && !ch->must_extract)
                            {
                                ch->trig_timer=NULL;
                                mprog_timer_init( ch );
                            }
                            break;

                        case GO_TYPE_OBJ:
                            obj=(OBJ_DATA *)(tmr->game_obj);
                            if (!valid_OBJ( obj ) )
                            {
                                bugf("timer_update: invalid obj");
                                break;
                            }
                            op_timer_trigger( obj );
                            if (valid_OBJ( obj ) && !obj->must_extract)
                            {
                                obj->otrig_timer=NULL;
                                oprog_timer_init( obj );
                            }
                            break;

                        case GO_TYPE_AREA:
                            /* no need for valid check on areas */
                            area=(AREA_DATA *)(tmr->game_obj);
                            ap_timer_trigger( area );
                            area->atrig_timer=NULL;
                            aprog_timer_init( area );
                            break;

                        case GO_TYPE_ROOM:
                            room=(ROOM_INDEX_DATA *)(tmr->game_obj);
                            if (!valid_ROOM( room ))
                            {
                                bugf("timer_update: invalid room");
                                break;
                            }
                            rp_timer_trigger( room );
                            room->rtrig_timer=NULL;
                            rprog_timer_init( room );
                            break;

                        default:
                            bugf("Invalid type in timer update: %d.", tmr->go_type);
                            remove_timer(tmr);
                            return;
                    }
                    break;
                case TM_LUAFUNC:
                    run_delayed_function(tmr);
                    break;
                default:
                    bugf("Invalid timer type: %d", tmr->tm_type);
                    remove_timer(tmr);
                    return;
            }
            /* it fired, kill it */
            remove_timer( tmr );
        }
    }
}

char * print_timer_list()
{
    static char buf[MSL*4];
    TIMER_NODE *tmr;
    strcpy(buf, "");
    int i=1;
    for ( tmr=first_timer; tmr; tmr=tmr->next )
    {
        if ( tmr->tm_type==TM_PROG && !valid_UD( tmr->game_obj ) )
        {
            bugf("Invalid game_obj in print_timer_list.");
            continue;
        } 
        sprintf(buf, "%s\n\r%d %s %d %s", buf, i,
            tmr->tm_type == TM_LUAFUNC ? "luafunc" :
            tmr->go_type == GO_TYPE_CH ? ((CHAR_DATA *)(tmr->game_obj))->name :
            tmr->go_type == GO_TYPE_OBJ ? ((OBJ_DATA *)(tmr->game_obj))->name :
            tmr->go_type == GO_TYPE_AREA ? ((AREA_DATA *)(tmr->game_obj))->name :
            tmr->go_type == GO_TYPE_ROOM ? ((ROOM_INDEX_DATA *)(tmr->game_obj))->name :
            "unknown",
            tmr->current,
            tmr->tag ? tmr->tag : "none");
        i++;
    }
    strcat( buf, "\n\r");
    return buf;

}
