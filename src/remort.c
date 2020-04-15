#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "buffer_util.h"
#include "simsave.h"
#include "special.h"

DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_outfit );
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_visible );

typedef struct remort_table REMORT_TABLE;

struct remort_table
{
    REMORT_TABLE *next;
    const char *name;
    int remorts;
    time_t signup;
    time_t limit;	
};

struct remort_chamber
{
    const char *name;
    int vnum;
    int availability;
    bool speed;
};

#define R1  1
#define R2  2
#define R3  4
#define R4  8
#define R5  16
#define R6  32 // Astark added 12-21-12. Testing.
#define R7  64 // Astark added 12-21-12. Testing.
#define R8  128 // Astark added 12-22-12. Testing.
#define R9  256 // Enabled August 1 2014
#define R10 512 // Enabled September 4 2015 (go live expected Sept 5)

#define HOUR 3600
#define DAY (24*HOUR)
#define WEEK (7*DAY)


static const struct remort_chamber chambers[] =
{
    {"Remort: Eight Trials            ",   5350, R1+R2, FALSE},
    {"Remort: Eight Trials            ",   6600, R1+R2, FALSE},
    {"Remort: Eight Trials            ",   6650, R1+R2, TRUE},
    {"Remort: Afterlife               ",   3000, R3+R4, FALSE},
    {"Remort: Afterlife               ",   6800, R3+R4, FALSE},
    {"Remort: Afterlife               ",  22500, R3+R4, TRUE},
    {"Remort: Counterintuition        ",  13700,    R5, FALSE},
    {"Remort: Counterintuition        ",  27300,    R5, FALSE},
    {"Remort: Counterintuition        ",  27450,    R5, TRUE},
    {"Remort: Lost Library            ",    101,    R6, FALSE},
    {"Remort: Lost Library            ",  16700,    R6, FALSE},
    {"Remort: Lost Library            ",  24451,    R6, TRUE},
 /* Note that the starting vnums for remort 7 is not the first vnum for the
    area. This is because the zone was built with mapmaker, and it somehow
    assigned a weird vnum to the first room */
    {"Remort: Tribulations of Dakaria ",  31394,    R7, FALSE},
    {"Remort: Tribulations of Dakaria ",   4694,    R7, FALSE},
    {"Remort: Tribulations of Dakaria ",   3344,    R7, TRUE},
    {"Remort: Urban Wasteland         ",   9000,    R8, FALSE},
    {"Remort: Urban Wasteland         ",  18500,    R8, FALSE},
    {"Remort: Urban Wasteland         ",  18700,    R8, TRUE},
    {"Remort: Curse of the Ages       ",    300,    R9, FALSE},
    {"Remort: Curse of the Ages       ",   9800,    R9, FALSE},
    {"Remort: Curse of the Ages       ",  13500,    R9, FALSE},
    {"Remort: Curse of the Ages       ",  16500,    R9, TRUE},
    {"Remort: Sundered Plains         ",  30479,   R10, FALSE},
    {"Remort: Sundered Plains         ",  19979,   R10, FALSE},
    {"Remort: Sundered Plains         ",  20579,   R10, FALSE},
    {"Remort: Sundered Plains         ",  23579,   R10, TRUE},
    {NULL,			0, 0}
};

#define MAX_CHAMBER ((sizeof(chambers)/sizeof(chambers[0])) - 1)

static REMORT_TABLE *chamber_list[MAX_CHAMBER];
static REMORT_TABLE *wait_list;

const char *time_format(time_t t, char *b, size_t bsz)
{
    static const char * const error_rtn = "[[ERROR]]";

    if ( bsz < 25 )
    {
        bugf("%s: expecting buffer size 25 or higher", __func__);
        return error_rtn;
    }

    snprintf(b, bsz, "%s", (char *) ctime( &t ));
    b[24]='\0';
    return b;
}

bool is_in_remort(CHAR_DATA *ch)
{
    int i;
    for ( i = 0; chambers[i].name != NULL; i++ )
        if ( chamber_list[i] != NULL && !str_cmp(ch->name, chamber_list[i]->name) )
            return TRUE;
    return FALSE;
}

static void remort_signup args( (CHAR_DATA *ch, CHAR_DATA *adept) );
static void remort_cancel args( (CHAR_DATA *ch, CHAR_DATA *adept) );
static void remort_status args( (CHAR_DATA *ch, CHAR_DATA *adept) );
static void remort_enter args( (CHAR_DATA *ch, CHAR_DATA *adept) );
static void remort_speed args( (CHAR_DATA *ch, CHAR_DATA *adept) );
static void remort_repeat args( (CHAR_DATA *ch, CHAR_DATA *adept, const char *arg) );
static void remort_save args( ( void ) );

DEF_DO_FUN(do_remort)
{
    CHAR_DATA *adept;
    char arg [MAX_INPUT_LENGTH];
    
    if (IS_NPC(ch)) 
        return;
    
    argument = one_argument(argument, arg);
   
    for ( adept = ch->in_room->people; adept != NULL; adept = adept->next_in_room )
    {
        if (!IS_NPC(adept)) 
            continue;

        if (adept->spec_fun == spec_remort) 
            break;
    }

    if (IS_IMMORTAL(ch))
        adept = ch;

    if ((adept == NULL || adept->spec_fun != spec_remort) && !IS_IMMORTAL(ch))
    {
        send_to_char("There is no remort adept here.\n\r",ch);
        return;
    }
    
    if ( !can_see(adept, ch) && !IS_IMMORTAL(ch))
    {
        do_say(adept, "Whazzat?  Did I hear something?");
        return;
    }
    
    if (arg[0] != '\0')
    {
        if (!strcmp(arg, "signup"))
        {
            remort_signup(ch, adept);
            return;
        }
        else if (!strcmp(arg, "cancel"))
        {
            remort_cancel(ch, adept);
            return;
        }
        else if (!strcmp(arg, "status"))
        {
            remort_status(ch, adept);
            return;
        }
        else if (!strcmp(arg, "enter"))
        {
            remort_enter(ch, adept);
            return;
        }
        else if (!strcmp(arg, "speed"))
        {
            remort_speed(ch, adept);
            return;
        }
        else if (!strcmp(arg, "repeat"))
        {
            remort_repeat(ch, adept, argument);
            return;
        }
    }
    
    send_to_char("Remort options: signup, cancel, status, enter, speed, repeat.\n\r", ch);
    send_to_char("For more information, type 'HELP REMORT'.\n\r", ch);
}

// remort is 1..10
static int remort_cost_gold(int remort)
{
#ifdef TESTER
    return 1;
#else
    return 1000 * (1 << remort/2) * (remort % 2 ? 14 : 10);
#endif
}

static int remort_cost_qp(int remort)
{
#ifdef TESTER
    return 1;
#else
    return 150 + 50 * remort;
#endif
}

static void remort_signup(CHAR_DATA *ch, CHAR_DATA *adept)
{
    REMORT_TABLE *new; 
    REMORT_TABLE *i;
    int qpcost, goldcost;
    char buf[MAX_STRING_LENGTH];
    int j, found;

    for (j=0, found=0; chambers[j].name; j++)
        if (chambers[j].availability & (1<<(ch->pcdata->remorts)))
        {
            found = 1;
            break;
        }

    if (found==0)
    {
        snprintf( buf, sizeof(buf), "You are too powerful for that, %s.", ch->name);
        do_say(adept, buf);
        return;
    }

    if (ch->level < 90 + ch->pcdata->remorts)
    {
        snprintf( buf, sizeof(buf), "You arent ready to leave this body behind, %s.", ch->name);
        do_say(adept, buf);
        return;
    }

    for (i = wait_list; i != NULL; i = i->next)
    {
        if (!str_cmp(ch->name, i->name))
        {
            snprintf( buf, sizeof(buf), "You are already on the list, %s.", ch->name);
            do_say(adept, buf);
            return;
        }
    }

    qpcost = remort_cost_qp(ch->pcdata->remorts + 1);
    goldcost = remort_cost_gold(ch->pcdata->remorts + 1);

    if (ch->pcdata->questpoints < qpcost)
    {
        snprintf( buf, sizeof(buf), "You need %d quest points to get into remort, %s.", qpcost, ch->name);
        do_say(adept, buf);
        return;
    }

    if (ch->gold<goldcost)
    {
        snprintf( buf, sizeof(buf), "There is a %d gold remort tax, %s.", goldcost, ch->name);
        do_say(adept, buf);
        return;
    }

    snprintf( buf, sizeof(buf), "That'll be %d gold, and %d qps, %s.", goldcost, qpcost, ch->name);
    do_say(adept, buf);

    ch->pcdata->questpoints -= qpcost;
    ch->gold -= goldcost;

    new = alloc_mem(sizeof(REMORT_TABLE));
    new->next = NULL;
    new->name = str_dup(ch->name);
    new->remorts = ch->pcdata->remorts;
    new->signup = current_time;
    new->limit = 0;

    if ((i = wait_list) != NULL)
    {
        for(;i->next != NULL;) 
            i = i->next;
        i->next = new;
    }
    else
        wait_list = new;

    do_say(adept, "You are now on the list.");
    logpf( "%s signed up for remort for %d qp and %d gold",
            ch->name, qpcost, goldcost );

    remort_update();
}


static void remort_cancel(CHAR_DATA *ch, CHAR_DATA *adept)
{
    REMORT_TABLE *prev = NULL; 
    REMORT_TABLE *i;
    int qpcost, goldcost;
    char buf[MAX_STRING_LENGTH];
    bool found=FALSE;
    
    qpcost = remort_cost_qp(ch->pcdata->remorts + 1);
    goldcost = remort_cost_gold(ch->pcdata->remorts + 1);
    
    for (i = wait_list; i != NULL; i = i->next)
    {
        if (!str_cmp(ch->name, i->name))
        {
            found = TRUE;
            break;
        }
        prev = i;
    }
    
    if (!found)
    {
        snprintf( buf, sizeof(buf), "You aren't on the list, %s.", ch->name);
        do_say(adept, buf);
        return;
    }
    
    snprintf( buf, sizeof(buf), "You may have your %d gold, and %d qps back, %s.",
        goldcost, qpcost, ch->name);
    do_say(adept, buf);
    
    ch->pcdata->questpoints += qpcost;
    ch->gold += goldcost;
    
    if (i == wait_list)
    {
        wait_list = wait_list->next;
    }
    else
        prev->next = i->next;
    
    free_string(i->name);
    free_mem(i, sizeof(REMORT_TABLE));
    
    do_say(adept, "You are no longer on the list.");
    
    logpf( "%s canceled remort for %d qp and %d gold",
	   ch->name, qpcost, goldcost );

    remort_update();
}


static void remort_status(CHAR_DATA *ch, CHAR_DATA *adept)
{
    REMORT_TABLE *i;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char tbuf[MAX_STRING_LENGTH];
    int j;
    
    for (j = 0; chambers[j].name != NULL; j++)
    {
        
        if (chamber_list[j] != NULL)
        {
            snprintf( buf2, sizeof(buf2), "%15s  %s", chamber_list[j]->name,
                time_format(chamber_list[j]->limit, tbuf, sizeof(tbuf)));
        }
        else
            snprintf( buf2, sizeof(buf2), "          Nobody");
        
        snprintf( buf, sizeof(buf), "%s %s", chambers[j].name, buf2);

        do_say(adept, buf);
    }

    do_say(adept, "People still waiting for a chamber:");
    
    for (i = wait_list; i != NULL; i = i->next)
    {
        if (i->limit != 0)
        {
            snprintf( buf, sizeof(buf), "%15s %d  *%s", i->name, i->remorts,
                time_format(i->limit, tbuf, sizeof(tbuf)));
        }
        else
        {
            snprintf( buf, sizeof(buf), "%15s %d    %s", i->name, i->remorts,
                time_format(i->signup, tbuf, sizeof(tbuf)));
        }
        
        do_say(adept, buf);
    }
}

static void remort_reset_area( AREA_DATA *pArea )
{
    int i;
    purge_area( pArea );
    if ( IS_SET(pArea->area_flags, AREA_NOREPOP) )
    {
	/* reset several times to load several mobs in a room */
	for ( i = 0; i < 5; i++ )
	    reset_area( pArea );
    }
    else
	reset_area( pArea );
}

static void remort_enter(CHAR_DATA *ch, CHAR_DATA *adept)
{
    REMORT_TABLE *prev=NULL; 
    REMORT_TABLE *i;
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int j;
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *room;
    
    for (i = wait_list; i != NULL; i =i->next)
    {
        if (!str_cmp(ch->name, i->name))
        {
            found = TRUE;
            break;
        }
        prev = i;
    }
    
    if (!found)
    {
        snprintf( buf, sizeof(buf), "You aren't on the list, %s.", ch->name);
        do_say(adept, buf);
        return;
    }
    
    if (i->limit == 0)
    {
        snprintf( buf, sizeof(buf), "I'm sorry, there are no chambers available for you, %s.", ch->name);
        do_say(adept, buf);
        return;
    }
    
    if ( carries_obj_recursive(ch, &is_questeq) )
    {
        snprintf( buf, sizeof(buf), "You'll want to leave your quest equipment somewhere safe, %s.", ch->name);
        do_say(adept, buf);
        return;
    }
    
    if (i == wait_list)
    {
        wait_list = wait_list->next;
    }
    else
        prev->next = i->next;
    
    for (j = 0; chambers[j].name != NULL; j++)
    {
        if (chamber_list[j] != NULL)
            continue;
        
        if (chambers[j].availability & (1<<(ch->pcdata->remorts)))
        {
            chamber_list[j] = i;
            break;
        }
    }
    
    if ( (room = get_room_index(chambers[j].vnum)) == NULL )
    {
	bugf( "remort_enter: room %d doesn't exists", chambers[j].vnum );
	send_to_char( "Bug: please contact an immortal!\n\r", ch );
	return;
    }

    /* reset the area */
    pArea = get_vnum_area( chambers[j].vnum );
    clear_area_quests( ch, pArea );
    //purge_area( pArea );
    remort_reset_area( pArea );

    act( "$n vanishes into a shimmering vortex.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, room );
    send_to_char("You step into a shimmering vortex and arrive in another dimension.\n\r",ch);
    do_look( ch, "auto" );
    make_visible( ch );
    affect_strip( ch, gsn_god_bless );
    affect_strip( ch, gsn_god_curse );
    die_follower(ch, FALSE);
    
    i->limit = current_time + WEEK;
    
    remort_update();
}


static void remort_speed(CHAR_DATA *ch, CHAR_DATA *adept)
{
    REMORT_TABLE *prev=NULL; 
    REMORT_TABLE *i;
    char buf[MAX_STRING_LENGTH];
    bool found=FALSE;
    int j;
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *room;
    
    for (i = wait_list; i != NULL; i = i->next)
    {
        if (!str_cmp(ch->name, i->name))
        {
            found = TRUE;
            break;
        }
        prev = i;
    }
    
    if (!found)
    {
        snprintf( buf, sizeof(buf), "You aren't on the list, %s.", ch->name);
        do_say(adept, buf);
        return;
    }
    
    if ( carries_obj_recursive(ch, &is_questeq) )
    {
        snprintf( buf, sizeof(buf), "You'll want to leave your quest equipment somewhere safe, %s.", ch->name);
        do_say(adept, buf);
        return;
    }
    
    for (j = 0; chambers[j].name != NULL; j++)
    {
        if (chamber_list[j] != NULL)
            continue;
        
        if ((chambers[j].availability & (1<<(ch->pcdata->remorts))) && chambers[j].speed)
        {
            chamber_list[j] = i;
            break;
        }
    }
    
    if (chambers[j].name == NULL)
    {
        snprintf( buf, sizeof(buf), "There are no speed chambers ready, %s.  Keep your pants on.", ch->name);
        do_say(adept, buf);
        WAIT_STATE( ch, PULSE_VIOLENCE );
        return;
    }
    
    if ( (room = get_room_index(chambers[j].vnum)) == NULL )
    {
	bugf( "remort_speed: room %d doesn't exists", chambers[j].vnum );
	send_to_char( "Bug: please contact an immortal!\n\r", ch );
	return;
    }

    if (i == wait_list)
    {
        wait_list = wait_list->next;
    }
    else
        prev->next = i->next;
    
    /* reset the area */
    pArea = get_vnum_area( chambers[j].vnum );
    clear_area_quests( ch, pArea );
    //purge_area( pArea );
    remort_reset_area( pArea );

    act( "$n vanishes into a shimmering vortex.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, room );
    send_to_char("You step into a shimmering vortex and arrive in another dimension.\n\r",ch);
    do_look( ch, "auto" );
    make_visible( ch );
    affect_strip( ch, gsn_god_bless );
    affect_strip( ch, gsn_god_curse );
    die_follower(ch, FALSE);
    
    i->limit = current_time + 8*HOUR;
    
    remort_update();
}


void remort_update( void )
{
    PERF_PROF_ENTER( pr_, "remort_update" );

    REMORT_TABLE *i;
    REMORT_TABLE *prev = NULL;
    AREA_DATA *pArea;
    bool found = FALSE;
    int j;
    CHAR_DATA *ch = NULL;
    DESCRIPTOR_DATA *d;
    bool used[MAX_CHAMBER];

    for (j = 0; chambers[j].name != NULL; j++)
        if (chamber_list[j] != NULL &&
                (chamber_list[j]->limit < current_time))
        {
            char log_buf1[MSL];

            snprintf( log_buf1, sizeof(log_buf1), "%s has run out of time for remort", chamber_list[j]->name );
            log_string( log_buf1 );

            for ( d = descriptor_list; d != NULL; d = d->next )
            {
                ch = d->character;
                if ((IS_PLAYING(d->connected)) 
                        && !str_cmp(ch->name, chamber_list[j]->name))
                {
                    found = TRUE;
                    break;
                }
            }

            if (found)
            {
                send_to_char("You have run out of time to complete remort.\n\r",ch);
                extract_char_eq( ch, &is_remort_obj, -1 );
                char_from_room( ch );
                char_to_room( ch, get_room_index(ROOM_VNUM_RECALL) );
                do_look( ch, "auto" );
            }
            else
            {
                d = new_descriptor();
                load_char_obj(d, chamber_list[j]->name, FALSE);

                if (d->character != NULL)
                {
                    d->character->in_room = get_room_index(ROOM_VNUM_RECALL);
                    extract_char_eq( d->character, &is_remort_obj, -1 );
                }
                quit_save_char_obj(d->character);
                /* load_char_obj still loads "default" character
                   even if player not found, so need to free it */
                if (d->character)
                {
                    nuke_pets(d->character);
                    free_char(d->character);
                    d->character=NULL;
                }
                free_descriptor(d);
            }

            free_string(chamber_list[j]->name);
            free_mem(chamber_list[j], sizeof(REMORT_TABLE));
            chamber_list[j] = NULL;

            pArea = get_vnum_area(chambers[j].vnum);
            purge_area(pArea);
            reset_area(pArea);

            remort_save();
        }

    for (i = wait_list; i != NULL; i = i->next)
    {
        if (i->limit && i->limit < current_time)
        {
            if (i == wait_list)
                wait_list = i->next;
            else
                prev->next = i->next;

            free_string(i->name);
            free_mem(i, sizeof(REMORT_TABLE));
            break;
        }

        prev = i;
    }

    for (j = 0; chambers[j].name != NULL; j++)
        used[j] = (chamber_list[j] != NULL);

    for (i = wait_list; i != NULL; i = i->next)
    {
        for (j = 0; chambers[j].name != NULL; j++)
            if ( (chambers[j].availability & (1<<(i->remorts)) )
                    && used[j] == FALSE && chambers[j].speed == FALSE)
            {
                used[j] = TRUE;
                if (i->limit == 0)
                    i->limit = current_time + WEEK;
                break;
            }
    }

    remort_save();

    PERF_PROF_EXIT( pr_ );
} 


void remort_remove(CHAR_DATA *ch, bool success)
{
    int j;
    AREA_DATA *pArea;
    char log_buf1[MSL];

    if (IS_NPC(ch) || ch->level<90)
	return;
    
    for (j = 0; chambers[j].name != NULL; j++)
        if (chamber_list[j] != NULL)
            if (!str_cmp(ch->name, chamber_list[j]->name))
            {
		snprintf( log_buf1, sizeof(log_buf1), "%s removed from remort after %s",
			 ch->name, success ? "completion" : "failure" );
		log_string( log_buf1 );

                free_string(chamber_list[j]->name);
                free_mem(chamber_list[j], sizeof(REMORT_TABLE));
                chamber_list[j]=NULL;
                pArea = get_vnum_area(chambers[j].vnum);
		clear_area_quests( ch, pArea );
		extract_char_eq( ch, &is_remort_obj, -1 );
                purge_area(pArea);
                reset_area(pArea);
                remort_save();
        // reimburst half cost
        if ( !success )
        {
            int reimb_qp = remort_cost_qp(ch->pcdata->remorts + 1) / 2;
            int reimb_gold = remort_cost_gold(ch->pcdata->remorts + 1) / 2;
            ch->pcdata->questpoints += reimb_qp;
            ch->pcdata->bank += reimb_gold;
            logpf("%s has been reimbursed %d qps and %d gold", ch->name, reimb_qp, reimb_gold);
            printf_to_char(ch, "You have been reimbursed %d qps, and %d gold has been deposited into your bank account.\n\r", reimb_qp, reimb_gold);
        }
		/* clear all money char holds */
		ch->gold = 0;
		ch->silver = 0;
		break;
            }
}


void remort_load( void )
{
    FILE *fp;
    REMORT_TABLE *p, *q = NULL;
    const char *s;
    int i;
    
    wait_list = NULL;
    
    fp = fopen (REMORT_FILE, "r");
    
    if (!fp)
    {
        for (i = 0; chambers[i].name != NULL; i++)
            chamber_list[i] = NULL;
        
        return;
    }
    
    for (i = 0; chambers[i].name != NULL; i++)
    {
        s = str_dup(fread_word (fp));
	/* safety-net in case of newly added remort-chambers */
	if ( s && str_cmp(s, "END") )
	    if ( str_cmp(s, "X") )
	    {
		p = alloc_mem(sizeof(REMORT_TABLE));
		p->name = s;
		p->remorts = fread_number(fp);
		p->signup = fread_number(fp);
		p->limit = fread_number(fp);
		chamber_list[i] = p;
	    }
	    else
	    {
		chamber_list[i] = NULL;
		free_string(s);
	    }
	else
	{
	    for ( ; chambers[i].name != NULL; i++)
		chamber_list[i] = NULL;
	    free_string(s);
	    fclose (fp);
	    return;
	}
    }
    
    s = str_dup(fread_word (fp));
    while (s && str_cmp(s, "END"))
    {
	/* safety-net in case remort chambers got removed */
	if ( str_cmp(s, "X") )
	{
	    p = alloc_mem(sizeof(REMORT_TABLE));

	    if (wait_list == NULL)
		wait_list = p;
	    else
		q->next = p;

	    q = p;
	    p->name = s;
	    p->next = NULL;
	    p->remorts = fread_number(fp);
	    p->signup = fread_number(fp);
	    p->limit = fread_number(fp);
        }
	else
	    free_string(s);  

	s = str_dup(fread_word (fp));
    }
    free_string(s);
    
    fclose (fp);		
}

/* toggle wether remort should be saved with next autosave
 */
static bool remort_save_needed = FALSE;
static void remort_save( void )
{
    remort_save_needed = TRUE;
}

/* save to memory, not to disk 
 */
MEMFILE* remort_mem_save( void )
{
    PERF_PROF_ENTER( pr_, "remort_mem_save" );

    REMORT_TABLE *p;
    int i;
    MEMFILE *mf;
    DBUFFER *buf;

#if defined(SIM_DEBUG)
    log_string("remort_mem_save: start");
#endif

    if ( !remort_save_needed )
    {
        PERF_PROF_EXIT( pr_ );
        return NULL;    
    }
	

    mf = memfile_new( REMORT_FILE, 1024 );
    
    if (mf == NULL)
    {
      bug("remort_mem_save: out of memory", 0);
      PERF_PROF_EXIT( pr_ );
      return NULL;
    }

    buf = mf->buf;
    for (i = 0; chambers[i].name != NULL; i++)
    {
        if (chamber_list[i] == NULL)
            bprintf (buf, "X\n");
        else
            bprintf (buf, "%s %d %ld %ld\n", 
            chamber_list[i]->name,
            chamber_list[i]->remorts,
            chamber_list[i]->signup,
            chamber_list[i]->limit);
    }
    
    for (p = wait_list; p ; p = p->next)
        bprintf (buf, "%s %d %ld %ld\n", p->name, p->remorts, p->signup, p->limit);
    
    bprintf (buf, "END\n");

    /* check for overflow */
    if (buf->overflowed)
    {
      bug("remort_mem_save: buffer overflow", 0);
      memfile_free(mf);
      PERF_PROF_EXIT( pr_ );
      return NULL;
    }
    
#if defined(SIM_DEBUG)
   log_string("remort_mem_save: done");
#endif

   remort_save_needed = FALSE;
   PERF_PROF_EXIT( pr_ );
   return mf;
}

int subclass_count( int class )
{
    int sc, count = 0;
    for ( sc = 1; subclass_table[sc].name != NULL; sc++ )
        if ( can_take_subclass(class, sc) )
            count++;
    return count;
}

void remort_begin(CHAR_DATA *ch)
{
    int i;
    bool reconnect = IS_SET(ch->act, PLR_REMORT_ROLL);

    remort_remove(ch, TRUE);

    // mark as rolling stats in case we lose connection
    if ( !reconnect )
    {
        SET_BIT(ch->act, PLR_REMORT_ROLL);
        quit_save_char_obj(ch);
    }
    
    if (ch->desc != NULL)
    {
        ch->desc->connected = CREATION_REMORT * MAX_CON_STATE + CON_REMORT_BEGIN;
    }
    else
    {
        do_quit(ch, "");
        return;
    }
    
    if ( !reconnect )
    {
        char_from_char_list(ch);
    
        if ( is_in_room(ch) )
            char_from_room(ch);
        
        /* need to do a little cleanup*/
        CHAR_DATA *wch;
        for ( wch = char_list; wch != NULL; wch = wch->next )
        {
            if ( wch->reply == ch )
                wch->reply = NULL;
            if ( ch->mprog_target == wch )
                wch->mprog_target = NULL;
        }
    }

    for (i = 0; i < MAX_STATS; i++)
        ch->pcdata->history_stats[i] += ch->pcdata->original_stats[i];
    
    send_to_char("{CYou are gently pulled upward into a column of brilliant\n\r", ch);
    send_to_char("light.  As you look down, you can see your physical body\n\r", ch);
    send_to_char("lying prone, lifeless.  As if alive beneath the divine\n\r", ch);
    send_to_char("radiance, the earth absorbs your spent body into itself.\n\r", ch);
    send_to_char("Unencumbered by a physical anchor, your spirit is free\n\r", ch);
    send_to_char("to roam all creation until you next choose to take on a\n\r", ch);
    send_to_char("material form.{g{ \n\r\n\r", ch);
    send_to_char("              (Press Enter to Continue)\n\r", ch);
    
}


void remort_complete(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj, *obj_next;

    REMOVE_BIT(ch->act, PLR_REMORT_ROLL);
    REMOVE_BIT(ch->act, PLR_REMORT_REPEAT);
    
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;
        extract_obj( obj );
    }
    
    ch->level = 1;
    ch->pcdata->highest_level = 1;
    ch->gold = 0;
    ch->silver = 0;
    ch->pcdata->points = 50;
    ch->exp = exp_per_level(ch);
    
    if (ch->pcdata->remorts == 0)
    {
        /* ascent */
        ch->pcdata->mob_deaths_ascent = 0;
    }
    ch->pcdata->mob_deaths_remort = 0;

    ch->train    = ch->train/2 + 2;
    ch->practice = ch->practice/2 + 10;

    /*
    ch->pcdata->trained_hit = 0;
    ch->pcdata->trained_move = 0;
    ch->pcdata->trained_mana = 0;
    */
    ch->pcdata->trained_hit  /= 2;
    ch->pcdata->trained_move /= 2;
    ch->pcdata->trained_mana /= 2;

    reset_char(ch);
    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->move = ch->max_move;

    ch->pcdata->field = 0;

    ch->pcdata->condition[0] = 0;
    ch->pcdata->condition[1] = 0;
    ch->pcdata->morph_time = 0;
    ch->pcdata->morph_race = 0;
    reset_char(ch);
    
    if (IS_SET(ch->form, FORM_CONSTRUCT))
    {
	ch->pcdata->condition[2] = -1;
	ch->pcdata->condition[3] = -1;
    }
    else
    {
	ch->pcdata->condition[2] = 72;
	ch->pcdata->condition[3] = 72;
    }
    ch->pcdata->condition[4] = 0;
    ch->stance = 0;
    ch->song = 0;
    
    if (! IS_SET(ch->act, PLR_TITLE))
    {
        snprintf( buf, sizeof(buf), "the %s", title_table [ch->clss] [1]);
        set_title( ch, buf );
    }
    
    do_outfit(ch,"");
    obj_to_char(create_object_vnum(OBJ_VNUM_MAP), ch);
    
    char_list_insert(ch);
    
    ch->desc->connected = CON_PLAYING;
    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
    die_follower(ch, false);
    send_to_char("\n\r",ch);
    
    snprintf( buf, sizeof(buf), "After much struggle, %s has made it to level 1!", ch->name);
    info_message(ch, buf, FALSE);

    force_full_save();
}

static void remort_repeat( CHAR_DATA *ch, CHAR_DATA *adept, const char *arg )
{
    char buf[MSL];

    if ( !IS_HERO(ch) )
    {
        send_to_char( "You haven't reached your maximum level yet.\n\r", ch );
        return;
    }

       
    if ( ch->pcdata->remorts < MAX_REMORT )
    {
        send_to_char( "You haven't reached the maximum remort level yet.\n\r", ch );
        send_to_char( "To advance to the next remort level, use <remort signup>.\n\r", ch );
        if ( ch->pcdata->remorts < 1 && ch->pcdata->ascents < 1 )
            return;
    }

    // half cost of initial remort
    int qpcost = remort_cost_qp(ch->pcdata->remorts) / 2;
    int goldcost = remort_cost_gold(ch->pcdata->remorts) / 2;
    // after ascension, remort repeats are initially free to allow experimentation
    if ( ch->pcdata->ascents > 0 && ch->pcdata->remorts <= subclass_count(ch->clss) )
        qpcost = goldcost = 0;
    
    if ( strcmp(arg, "confirm") )
    {
        send_to_char( "To repeat your current remort, type <remort repeat confirm>.\n\r", ch );
        printf_to_char( ch, "WARNING: This will cost %d qp and %d gold!\n\r", qpcost, goldcost );
        return;
    }

    if ( ch->pcdata->questpoints < qpcost )
    {
        snprintf( buf, sizeof(buf), "You need %d quest points to repeat remort, %s.", qpcost, ch->name);
        do_say(adept, buf);
        return;
    }
    
    if ( ch->gold < goldcost )
    {
        snprintf( buf, sizeof(buf), "There is a %d gold remort repetition tax, %s.", goldcost, ch->name);
        do_say(adept, buf);
        return;
    }
    
    if ( ch->carrying != NULL )
    {
        snprintf( buf, sizeof(buf), "You must leave all posessions behind, %s.", ch->name);
        do_say(adept, buf);
        return;
    }

    snprintf( buf, sizeof(buf), "That'll be %d gold, and %d qps, %s.", goldcost, qpcost, ch->name);
    do_say(adept, buf);

    ch->pcdata->questpoints -= qpcost;
    ch->gold -= goldcost;    
    logpf("Repeating remort for %s. Deducted %d qp and %d gold.", ch->name, qpcost, goldcost);

    SET_BIT(ch->act, PLR_REMORT_REPEAT);
    remort_begin(ch);
} 

DEF_DO_FUN(do_ascend)
{
    if ( IS_NPC(ch) )
        return;

    if ( ch->level < LEVEL_HERO || ch->pcdata->remorts < MAX_REMORT )
    {
        ptc(ch, "You need to reach level %d before you can ascend.\n\r", LEVEL_HERO);
#ifdef TESTER
        ptc(ch, "We will ignore that for testing though.\n\r");
#else
        return;
#endif
    }
    
    if ( ch->pcdata->religion_rank < RELIGION_MAX_RANK )
    {
        ptc(ch, "You need to reach the rank of %s before you can ascend.\n\r", get_religion_rank_name(RELIGION_MAX_RANK) );
#ifdef TESTER
        ptc(ch, "We will ignore that for testing though.\n\r");
#else
        return;
#endif
    }
    
    if ( strcmp(argument, "confirm") )
    {
        send_to_char("To ascend, type <ascend confirm>.\n\r", ch);
        send_to_char("WARNING: Doing so will cause you to be reborn as a remort 0 character!\n\r", ch);
        return;
    }

    if ( ch->carrying != NULL )
    {
        send_to_char("You must leave all posessions behind.\n\r", ch);
        return;
    }
    
    logpf("Ascending %s.", ch->name);
    ch->pcdata->remorts = 0;
    ch->pcdata->ascents += 1;
    
    remort_begin(ch);
}
