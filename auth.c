/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			 New Name Authorization support code                            *
 ****************************************************************************/

/*
 *  New name authorization system
 *  Author: Rantic (supfly@geocities.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 */

/*  
    This is a ROM 2.4b4 port of the name authorization code present in Smaug
    version 1.4.  Portions are derived from the stock Smaug code, portions are
    taken from Rantic's snippet which is intended to enhance the stock Smaug
    auth functionality, and portions were written by me and are unique to the
    ROM port.

    Author: Brian Castle ("Rimbol"), Aarchon MUD (darkhorse.triad.net 7000)
            March 1999
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#if !defined(WIN32)
#include <unistd.h>
#endif
#include <string.h>
#include "merc.h"


/* Function declarations */
bool check_parse_name	    args( ( char *name, bool newchar ) );
CHAR_DATA *get_waiting_desc args( ( CHAR_DATA *ch, char *name ) );

DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_reserve);

/* Local variables */
AUTH_LIST *first_auth_name;
AUTH_LIST *last_auth_name;

/* stuff for auto-authing recreating chars */
char last_delete_name[MIL] = "";

void add_auto_auth( char *name )
{
    sprintf( last_delete_name, "%s", name );
}

bool check_auto_auth( char *name )
{
    return str_cmp( last_delete_name, name ) == 0;
}

/* Will return TRUE if player is online or if a pfile exists by that name */
bool exists_player( char *name )
{
#if defined (WIN32)
    struct _stat fst;
#else
    struct stat fst;
#endif
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;

    
   for ( d = descriptor_list; d != NULL; d = d->next )
   {
	  victim = d->original ? d->original : d->character;

      if (!victim)
          continue;

      if (is_exact_name(name, victim->name))
          return TRUE;
   }

   sprintf( buf, "%s%s", PLAYER_DIR, capitalize(name));
   
#if defined (WIN32)    
   if( _stat(buf, &fst) != -1 )
#else
   if( stat(buf, &fst) != -1 )
#endif
       return TRUE;
   else
       return FALSE;
}

void clear_auth_list()
{
    AUTH_LIST *auth, *nauth;
    
    for ( auth = first_auth_name; auth; auth = nauth )
    {
        nauth = auth->next;
        if ( !exists_player( auth->name ) )
        {
            if ( !auth->prev )
                first_auth_name  = auth->next;
            else
                auth->prev->next = auth->next;
            if ( !auth->next )
                last_auth_name   = auth->prev;
            else
                auth->next->prev = auth->prev;
            
            if( auth->authed_by )
                free_string(auth->authed_by);

            if( auth->change_by )
                free_string(auth->change_by);

            if( auth->denied_by )
                free_string(auth->denied_by);

            free_string(auth->name);
            free_mem(auth, sizeof(AUTH_LIST));
        }
    }
    save_auth_list();
}

void write_auth_file( FILE *fpout, AUTH_LIST *list )
{
    fprintf( fpout, "Name		%s~\n",	list->name );
    fprintf( fpout, "State		%d\n", list->state );
    
    if (list == NULL)
        return;

    if( list->authed_by )
        fprintf( fpout, "AuthedBy       %s~\n", list->authed_by );

    if( list->change_by )
        fprintf( fpout, "Change		%s~\n", list->change_by );

    if( list->denied_by )
        fprintf( fpout, "Denied		%s~\n", list->denied_by );

    fprintf( fpout, "End\n\n" );
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value ) if ( !str_cmp( word, literal ) ) { field  = value; fMatch = TRUE; break; }


void fread_auth( FILE *fp )
{
    AUTH_LIST *new_auth;
    bool fMatch;
    char *word;
    char buf[MAX_STRING_LENGTH];
    
    new_auth = alloc_mem(sizeof(AUTH_LIST));
    
    new_auth->authed_by = NULL;
    new_auth->change_by = NULL;
    new_auth->denied_by = NULL;
    
    for ( ;; )
    {
        word = feof( fp ) ? "End" : fread_word ( fp );
        fMatch = FALSE;
        switch( UPPER( word[0] ) )
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
            
        case 'A':
            KEY( "AuthedBy",	new_auth->authed_by,	fread_string( fp ) );
            break;
            
        case 'C':
            KEY( "Change",	new_auth->change_by,	fread_string( fp ) );
            break;
            
        case 'D':
            KEY( "Denied",	new_auth->denied_by,	fread_string( fp ) );
            break;
            
        case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                fMatch = TRUE;

                if ( !first_auth_name )
                    first_auth_name      = new_auth;
                else
                    last_auth_name->next = new_auth;

                new_auth->next      = NULL;
                new_auth->prev      = last_auth_name;
                last_auth_name      = new_auth;
                
                return;
            }
            break;
            
        case 'N':
            KEY( "Name",	new_auth->name,	fread_string( fp ) );
            break;
            
        case 'S':
            if ( !str_cmp( word, "State" ) )
            {
                new_auth->state = fread_number( fp );

                if ( new_auth->state == AUTH_ONLINE || new_auth->state == AUTH_LINK_DEAD )
                    /* Crash proofing. Can't be online when */
                    /* booting up. Would suck for do_auth   */
                    new_auth->state = AUTH_OFFLINE;

                fMatch = TRUE;
                break;
            }
            break;
        }
        if ( !fMatch )
        {
            sprintf( buf, "Fread_auth: no match: %s", word );
            bug( buf, 0 );
        }
    }
}

void save_auth_list()
{
    FILE *fpout;
    AUTH_LIST *list;
    
    fclose(fpReserve);

    if ( ( fpout = fopen( AUTH_FILE, "w" ) ) == NULL )
    {
        bug( "Cannot open auth.txt for writing.", 0 );
        log_error( AUTH_FILE );
        fpReserve = fopen( NULL_FILE, "r" );
        return;
    }
    
    for ( list = first_auth_name; list; list = list->next )
    {
        fprintf( fpout, "#AUTH\n" );
        write_auth_file( fpout, list );
    } 
    
    fprintf( fpout, "#END\n" );
    fclose( fpout );
    fpout = NULL;

    fpReserve = fopen( NULL_FILE, "r" );
}

void load_auth_list()
{
    FILE *fp;
    int x;
    
    first_auth_name = last_auth_name = NULL;
    
    if ( ( fp = fopen( AUTH_FILE, "r" ) ) != NULL )
    {
        x = 0;
        for ( ;; )
        {
            char letter;
            char *word;
            
            letter = fread_letter( fp );
            if ( letter == '*' )
            {
                fread_to_eol( fp );
                continue;
            }
            
            if ( letter != '#' )
            {
                bug( "Load_auth_list: # not found.", 0 );
                break;
            }
            
            word = fread_word( fp );
            if ( !str_cmp( word, "AUTH" ) )
            {
                fread_auth( fp );
                continue;
            }
            else
                if ( !str_cmp( word, "END" ) )
                    break;
                else
                {
                    bug( "load_auth_list: bad section.", 0 );
                    continue;
                }
        }
        fclose( fp );
        fp = NULL;
    }
    else
    {
        bug( "Cannot open auth list file", 0 );
        exit( 0 );
    }
    clear_auth_list();
}


int get_auth_state( CHAR_DATA *ch )
{
    AUTH_LIST *namestate;
    int state;
    
    state = AUTH_AUTHED;
    
    for ( namestate = first_auth_name; namestate; namestate = namestate->next )
    {
        if ( !str_cmp( namestate->name, ch->name ) )
        {
            state = namestate->state;		
            break;
        }
    }
    return state;
}


AUTH_LIST *get_auth_name( char *name )
{
    AUTH_LIST *mname;
    
    if( last_auth_name && last_auth_name->next != NULL )
        bugf( "Last_auth_name->next != NULL: %s", last_auth_name->next->name );
    
    for( mname = first_auth_name; mname; mname = mname->next )
    {
        if ( !str_cmp( mname->name, name ) ) /* If the name is already in the list, break */
            break;
    }
    return mname;
}


void add_to_auth( CHAR_DATA *ch )
{
    AUTH_LIST *new_name;
    
    new_name = get_auth_name( ch->name );
    if ( new_name != NULL )
        return;
    else
    {
        new_name = alloc_mem(sizeof(AUTH_LIST));
        new_name->name = str_dup( ch->name );
        new_name->state = AUTH_ONLINE;        /* Just entered the game */
        new_name->authed_by = NULL;
        new_name->change_by = NULL;
        new_name->denied_by = NULL;

        if ( !first_auth_name )
            first_auth_name = new_name;
        else
            last_auth_name->next = new_name;

        new_name->next = NULL;
        new_name->prev = last_auth_name;
        last_auth_name = new_name;

        save_auth_list();
    }
}


void remove_from_auth( char *name )
{
    AUTH_LIST *old_name;
    
    old_name = get_auth_name( name );
    if ( old_name == NULL ) /* Its not old */
        return;
    else
    {
        if ( !old_name->prev )
            first_auth_name      = old_name->next;
        else
            old_name->prev->next = old_name->next;

        if ( !old_name->next )
            last_auth_name       = old_name->prev;
        else
            old_name->next->prev = old_name->prev;

        if( old_name->authed_by )
            free_string( old_name->authed_by );
        if( old_name->change_by )
            free_string( old_name->change_by );
        if( old_name->denied_by )
            free_string( old_name->denied_by );

        free_string( old_name->name );
        free_mem(old_name, sizeof(AUTH_LIST));

        save_auth_list();
    }
}


void check_auth_state( CHAR_DATA *ch )
{
    AUTH_LIST *old_auth;
    char buf[MAX_STRING_LENGTH];
    
    old_auth = get_auth_name( ch->name );

    if ( old_auth == NULL )
    {
	/* catch for invalid unauthed flags --Bobble */
	if ( !IS_NPC(ch) )
	    REMOVE_BIT( ch->act, PLR_UNAUTHED );
        return;
    }
    
    if ( old_auth->state == AUTH_OFFLINE || old_auth->state == AUTH_LINK_DEAD )
    {
        old_auth->state = AUTH_ONLINE;
        save_auth_list();
    }
    else if ( old_auth->state == AUTH_CHANGE_NAME )
    {
        printf_to_char(ch,
            "\n\r{RThe Immortals have found the name %s\n\r"
            "to be unacceptable. You must choose a new one.\n\r"
            "The name you choose must be clean and original.\n\r"
            "See 'help name'.{x\n\r", ch->name);
    }
    else if ( old_auth->state == AUTH_DENIED )
    {
	char filename[MIL];

        send_to_char( "{RYou have been denied access to the game.{x\n\r", ch );

        remove_from_auth( ch->name );

        sprintf( buf, "%s add", ch->name );
        do_reserve(ch, buf);

	sprintf( filename, "%s", capitalize(ch->name) );

        stop_fighting(ch,TRUE);
        do_quit( ch, "" );

        /*sprintf( buf, "%s%s", PLAYER_DIR, capitalize( ch->name ) );*/

        if ( !unlink_pfile(filename) )
        {
            sprintf(buf, "Pre-Auth %s denied. Player file destroyed.\n\r", filename );
            wiznet(buf, ch, NULL, WIZ_AUTH, 0, LEVEL_IMMORTAL);
        }
    }
    else if ( old_auth->state == AUTH_AUTHED )
    {
        if ( ch->pcdata->authed_by )
            free_string( ch->pcdata->authed_by );

        if( old_auth->authed_by )
        {
            ch->pcdata->authed_by = str_dup( old_auth->authed_by );
    
            /* Characters who were authorized while offline and/or linkdead */
	    /* no longer hang the MUD after they log back on, quit, and/or  */
	    /* delete.  "free_string(old_auth->authed_by);", below, was     */
	    /* replaced with "remove_from_auth(ch->name);", which seems to  */
	    /* do the trick - Elik, Jan 16, 2006 */  
	    /* 
	    free_string( old_auth->authed_by );
            */ 
	    remove_from_auth(ch->name);
        }
        else
            ch->pcdata->authed_by = str_dup("(Auto)");
        
        printf_to_char( ch,                                            
            "\n\r{GThe Immortals have accepted the name %s.\n\r"
            "You are now free to roam the MUD.{x\n\r", ch->name );

        REMOVE_BIT(ch->act, PLR_UNAUTHED);
        remove_from_auth( ch->name ); 
        return;
    }
    return;
}


bool is_waiting_for_auth( CHAR_DATA *ch )
{
    int state = get_auth_state( ch );
    return !IS_NPC(ch) && ch->desc
	&& (state == AUTH_ONLINE || state == AUTH_CHANGE_NAME)
	&& IS_SET(ch->act, PLR_UNAUTHED);
}

/* 
 * Check if the name prefix uniquely identifies a char descriptor
 */ 
CHAR_DATA *get_waiting_desc( CHAR_DATA *ch, char *name ) 
{ 
    DESCRIPTOR_DATA *d; 
    CHAR_DATA       *ret_char = NULL;
    static unsigned int number_of_hits; 
    
    number_of_hits = 0; 
    for ( d = descriptor_list; d; d = d->next ) 
    { 
        if ( d->character && (!str_prefix( name, d->character->name )) 
	     && is_waiting_for_auth(d->character) )
        { 
            if ( ++number_of_hits > 1 ) 
            { 
                printf_to_char( ch, "%s does not uniquely identify a char.\n\r", name ); 
                return NULL; 
            } 
            ret_char = d->character;       /* return current char on exit */
        } 
    }

    if ( number_of_hits == 1 ) 
        return ret_char; 
    else 
    { 
        send_to_char( "No one like that waiting for authorization.\n\r", ch ); 
        return NULL; 
    } 
} 


void do_authorize( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = NULL;
    AUTH_LIST *auth;
    /* offline doesn't seem to actually get used - Elik, Jan 16, 2006 */
    /*
    bool offline, authed, changename, denied, pending;
    */ 
    bool authed, changename, denied, pending;

    /* offline doesn't seem to actually get used - Elik, Jan 16, 2006 */
    /* 
    offline = authed = changename = denied = pending = FALSE;
    */ 
    authed = changename = denied = pending = FALSE;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* check which info to display */
    for ( auth = first_auth_name; auth; auth = auth->next )
    {
	if ( auth->state == AUTH_CHANGE_NAME )
	    changename = TRUE;
	else if ( auth->state == AUTH_AUTHED )
	    authed = TRUE;
	else if ( auth->state == AUTH_DENIED )
	    denied = TRUE;
	
	if ( auth->name != NULL && auth->state < AUTH_CHANGE_NAME)
	    pending = TRUE;
    }

    if ( arg1[0] == '\0' )
    {
        send_to_char( "To approve a waiting character:              auth <name>\n\r", ch );
        send_to_char( "To deny a waiting character:                 auth <name> deny\n\r", ch );
        send_to_char( "To ask a waiting character to change names:  auth <name> name\n\r", ch );
	send_to_char( "To get a list of authed or denied names:     auth list\n\r", ch );
        send_to_char( "To verify existence of players on the list:  auth cleanup\n\r", ch );
        send_to_char( "To toggle the auth system on/off:            auth toggle\n\r", ch);
        
        send_to_char("\n\r{+--- Characters awaiting approval ---{x\n\r", ch);
        

        if ( pending )
        {
            for ( auth = first_auth_name; auth; auth = auth->next )
            {
                if ( auth->state < AUTH_CHANGE_NAME )
                {
                    switch( auth->state )
                    {
                    default:
                        sprintf( buf, "Unknown?" );
                        break;
                    case AUTH_LINK_DEAD:
                        sprintf( buf, "Link Dead" );
                        break;
                    case AUTH_ONLINE:
                        sprintf( buf, "Online" );
                        break;
                    case AUTH_OFFLINE:
                        sprintf( buf, "Offline" );
                        break;
                    }
                    
                    printf_to_char( ch, "\t%s\t\t%s\n\r", auth->name, buf );
                }
            }
        }
        else
            send_to_char( "\tNone\n\r", ch );

	return;
    }

    if ( !str_cmp( arg1, "list" ) )
    {
	BUFFER *output = new_buf();

        if ( authed )
        {
            add_buf( output, "\n\r{+Authorized Characters:{x\n\r" );
            add_buf( output, "{+---------------------------------------------{x\n\r" );
            for ( auth = first_auth_name; auth; auth = auth->next )
            {
                if ( auth->state == AUTH_AUTHED )
		{
		    sprintf( buf, "Name: %s\t Approved by: %s\n\r", 
			     auth->name, auth->authed_by );
		    add_buf( output, buf );
		}
            }
        }
        if ( changename )
        {
            add_buf( output, "\n\r{+Change Name:{x\n\r" );
            add_buf( output, "{+---------------------------------------------{x\n\r" );
            for ( auth = first_auth_name; auth; auth = auth->next )
            {
                if ( auth->state == AUTH_CHANGE_NAME )
		{
                    sprintf( buf, "Name: %s\t Change requested by: %s\n\r", 
			     auth->name, auth->change_by );
 		    add_buf( output, buf );
		}
           }
        }
        if ( denied )
        {
            add_buf( output, "\n\r{+Denied Characters:{x\n\r" );
            add_buf( output, "{+---------------------------------------------{x\n\r" );
            for ( auth = first_auth_name; auth; auth = auth->next )
            {
                if ( auth->state == AUTH_DENIED )
		{
                    sprintf( buf, "Name: %s\t Denied by: %s\n\r", 
			     auth->name, auth->denied_by );
		    add_buf( output, buf );
		}
            }
        }

	page_to_char( buf_string(output), ch );
	free_buf(output);

        return;
    }

    if ( !str_cmp( arg1, "cleanup" ) )
    {
        send_to_char( "Checking authorization list...\n\r", ch );
        clear_auth_list();
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "toggle" ) )
    {
	if ( !IS_IMMORTAL(ch) )
	{
	    send_to_char( "Only immortals can toggle the authorization system.\n\r", ch );
	    return;
	}

        if (wait_for_auth == AUTH_STATUS_ENABLED)
        {
            send_to_char("Name authorization is now enabled when immortals are present.\n\r",ch);
            wait_for_auth = AUTH_STATUS_IMM_ON;
        }
        else if (wait_for_auth == AUTH_STATUS_IMM_ON)
        {
            send_to_char("Name authorization is now disabled.\n\r",ch);
            wait_for_auth = AUTH_STATUS_DISABLED;
        }
        else if (wait_for_auth == AUTH_STATUS_DISABLED)        
        {
            send_to_char("Name authorization is now active.\n\r",ch);
            wait_for_auth = AUTH_STATUS_ENABLED;
        }
        else
            wait_for_auth = AUTH_STATUS_DISABLED;
        return;
    }

    auth = get_auth_name( arg1 ); 

    if ( auth != NULL )
    {
	if ( auth->state == AUTH_OFFLINE || auth->state == AUTH_LINK_DEAD )
        {
            /* this offline = TRUE doesn't get used - Elik, Jan 16, 2006 */
	    /*
	    offline = TRUE;
            */ 
            if ( arg2[0]=='\0' || !str_cmp( arg2,"accept" ) || !str_cmp( arg2,"yes" ))
            {
                auth->state = AUTH_AUTHED;
                auth->authed_by = str_dup( ch->name );
                save_auth_list();
                sprintf( buf, "%s: authorized", auth->name);
                wiznet(buf, ch, NULL, WIZ_AUTH, 0, LEVEL_IMMORTAL);
                printf_to_char( ch, "You have authorized %s.\n\r", auth->name );
                return;
            }
            else if ( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
            {
                auth->state = AUTH_DENIED;
                auth->denied_by = str_dup( ch->name );
                save_auth_list();
                sprintf( buf, "%s: denied authorization", auth->name );
                wiznet(buf, ch, NULL, WIZ_AUTH, 0, LEVEL_IMMORTAL);
                printf_to_char( ch, "You have denied %s.\n\r", auth->name );

                sprintf( buf, "%s add", auth->name );
                do_reserve( ch, buf );
                return;
            }
            else if ( !str_cmp( arg2, "name" ) || !str_cmp(arg2, "n" ) )
            {
                auth->state = AUTH_CHANGE_NAME;
                auth->change_by = str_dup( ch->name );
                save_auth_list();
                sprintf( buf, "%s: name denied", auth->name );
                wiznet(buf, ch, NULL, WIZ_AUTH, 0, LEVEL_IMMORTAL);
                printf_to_char( ch, "You have requested %s to change names.\n\r", auth->name );

                sprintf( buf, "%s add", auth->name );
                do_reserve(ch, buf);
                return;
            }
            else
            {
                send_to_char("Invalid argument.\n\r", ch);
                return;
            }
        }
        else
        {
            victim = get_waiting_desc( ch, arg1 );
            if ( victim == NULL )
                return;
            
            if ( arg2[0]=='\0' || !str_cmp( arg2,"accept" ) || !str_cmp( arg2,"yes" ))
            {
		// check in case name got denied in the meantime
		if ( !check_parse_name(victim->name, TRUE) )
		{
		    send_to_char( "That name is not allowed.\n\r", ch );
		    return;
		}

                if ( victim->pcdata->authed_by )
                    free_string( victim->pcdata->authed_by );
                victim->pcdata->authed_by = str_dup( ch->name );
                sprintf( buf, "%s: authorized", victim->name );
                wiznet(buf, ch, NULL, WIZ_AUTH, 0, LEVEL_IMMORTAL);
                
                printf_to_char( ch, "You have authorized %s.\n\r", victim->name );
                
                printf_to_char( victim,
                    "\n\r{GThe Immortals have accepted the name %s.\n\r"
                    "You are now free to roam the MUD.{x\n\r", victim->name );

                REMOVE_BIT(victim->act, PLR_UNAUTHED);
                remove_from_auth( victim->name ); 
                return;
            }
            else if ( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
            {
		char filename[MIL];

                send_to_char( "{RYou have been denied access.{x\n\r", victim );
                sprintf( buf, "%s: denied authorization", victim->name );
                wiznet(buf, ch, NULL, WIZ_AUTH, 0, LEVEL_IMMORTAL);
                printf_to_char( ch, "You have denied %s.\n\r", victim->name );
                remove_from_auth( victim->name );

                sprintf( buf, "%s add", victim->name );
                do_reserve(ch, buf);

                /* Sardonic 10/99
                save_char_obj(victim);
                stop_fighting(victim,TRUE);
                do_quit( victim, "" );
                */

		sprintf( filename, "%s", capitalize(victim->name) );

                extract_char(victim, TRUE);
                if (victim->desc != NULL)
                    close_socket(victim->desc);
                
                /*sprintf( buf, "%s%s", PLAYER_DIR, capitalize( victim->name ) );*/
                
                if ( !unlink_pfile(filename) )
                {
                    sprintf(buf, "Pre-Auth %s denied. Player file destroyed.\n\r", filename );
                    wiznet(buf, victim, NULL, WIZ_AUTH, 0, LEVEL_IMMORTAL);
                }
            }	
            else if ( !str_cmp( arg2, "name" ) || !str_cmp(arg2, "n" ) )
            {
		if ( auth->state == AUTH_CHANGE_NAME )
		{
		    send_to_char( "They are already changing name.\n\r", ch );
		    return;
		}
                auth->state = AUTH_CHANGE_NAME;
                auth->change_by = str_dup( ch->name );
                save_auth_list();
                sprintf( buf, "%s: name denied", victim->name );
                wiznet(buf, victim, NULL, WIZ_AUTH, 0, LEVEL_IMMORTAL);

                printf_to_char(victim,
                    "\n\r{RThe Immortals have found the name %s\n\r"
                    "to be unacceptable. You must choose a new one.\n\r"
                    "The name you choose must be clean and original.\n\r"
                    "See 'help name'.{x\n\r", victim->name);

                printf_to_char( ch, "You have requested that %s change names.\n\r", victim->name);

                sprintf( buf, "%s add", victim->name );
                do_reserve(ch, buf);
                return;
            }
            else
            {
                send_to_char("Invalid argument.\n\r", ch);
                return;
            } 
        } 
    }
    else
    {
        send_to_char( "No such player pending authorization.\n\r", ch );
        return;
    }
}

/* new auth */
void do_name( CHAR_DATA *ch, char *argument )
{
    char fname[1024];
#if defined (WIN32)
    struct _stat fst;
#else
    struct stat fst;
#endif
    CHAR_DATA *tmp;
    AUTH_LIST *auth_name;
    
    auth_name = NULL;
    auth_name = get_auth_name( ch->name );
    if ( auth_name == NULL )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument[0] = UPPER(argument[0]);
    
    if (!check_parse_name(argument, TRUE))
    {
        send_to_char("Illegal name, try another.\n\r", ch);
        return;
    }
    
    if (!str_cmp(ch->name, argument))
    {
        send_to_char("That's already your name!\n\r", ch);
        return;
    }
    
    for ( tmp = char_list; tmp; tmp = tmp->next )
    {
        if (!str_cmp(argument, tmp->name))
            break;
    }
    
    if ( tmp )
    {
        send_to_char("That name is already taken.  Please choose another.\n\r", ch);
        return;
    }
    
    sprintf( fname, "%s%s", PLAYER_DIR, capitalize( argument ) );

#if defined (WIN32)
    if ( _stat( fname, &fst ) != -1 )
#else
    if ( stat( fname, &fst ) != -1 )
#endif
    {
        send_to_char("That name is already taken.  Please choose another.\n\r", ch);
        return;
    }
    /* sprintf( fname, "%s%s", PLAYER_DIR, capitalize(ch->name) ); */

    unlink_pfile( capitalize(ch->name) ); 
    
    free_string( ch->name );
    ch->name = str_dup( argument );
    send_to_char("Your name has been changed and is being submitted for approval.\n\r", ch );
    auth_name->name = str_dup( argument );
    auth_name->state = AUTH_ONLINE;

    if ( auth_name->change_by )
        free_string( auth_name->change_by );
    auth_name->change_by = NULL;
    save_auth_list();
    return;
}



void auth_update( void ) 
{ 
    AUTH_LIST *auth;
    char buf [MAX_STRING_LENGTH], log_buf [MAX_STRING_LENGTH];
    bool found_hit = FALSE;       /* was at least one found? */
    

    /* Auth notification no longer beeps - Elik, Jan 16, 2006. */
    /* beep imms.. */
    /* Readded the beep too many auths going unnoticed */
      strcpy( log_buf, "{*{+--- Characters awaiting approval ---{x\n\r" ); 
    /* strcpy(log_buf, "{+--- Characters awaiting approval ---{x\n\r"); */

    for ( auth = first_auth_name; auth; auth = auth->next )
    {
        if ( auth != NULL && auth->state < AUTH_CHANGE_NAME )
        {
            found_hit = TRUE;
            sprintf( buf, "Name: %s      Status: %s\n\r", auth->name, ( auth->state == AUTH_ONLINE ) ? "Online" : "Offline" ); 
            strcat( log_buf, buf );
        }
    }

    if ( found_hit )
        wiznet(log_buf, NULL, NULL, WIZ_AUTH, 0, LEVEL_IMMORTAL);
}

