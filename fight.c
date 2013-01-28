/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************/

/***************************************************************************
*   ROM 2.4 is copyright 1993-1996 Russ Taylor             *
*   ROM has been brought to you by the ROM consortium          *
*       Russ Taylor (rtaylor@efn.org)                  *
*       Gabrielle Taylor                           *
*       Brian Moore (zump@rom.org)                     *
*   By using this code, you have agreed to follow the terms of the     *
*   ROM license, in the file Rom24/doc/rom.license             *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "tables.h"
#include "warfare.h"
#include "lookup.h"

extern WAR_DATA war;

void reverse_char_list();
void check_rescue( CHAR_DATA *ch );
void check_jump_up( CHAR_DATA *ch );
void aura_damage( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield, int dam );
void stance_after_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield );
void weapon_flag_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield );
void check_behead( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield );
void handle_death( CHAR_DATA *ch, CHAR_DATA *victim );
void set_pos( CHAR_DATA *ch, int position );
void set_fighting_new( CHAR_DATA *ch, CHAR_DATA *victim, bool kill_trigger );
void adjust_pkgrade( CHAR_DATA *killer, CHAR_DATA *victim, bool theft );
void adjust_wargrade( CHAR_DATA *killer, CHAR_DATA *victim );

/* command procedures needed */
DECLARE_DO_FUN(do_backstab  );
DECLARE_DO_FUN(do_circle    );
DECLARE_DO_FUN(do_emote     );
DECLARE_DO_FUN(do_berserk   );
DECLARE_DO_FUN(do_bash      );
DECLARE_DO_FUN(do_trip      );
DECLARE_DO_FUN(do_dirt      );
DECLARE_DO_FUN(do_flee      );
DECLARE_DO_FUN(do_kick      );
DECLARE_DO_FUN(do_disarm    );
DECLARE_DO_FUN(do_get       );
DECLARE_DO_FUN(do_recall    );
DECLARE_DO_FUN(do_yell      );
DECLARE_DO_FUN(do_sacrifice );
DECLARE_DO_FUN(do_gouge     );
DECLARE_DO_FUN(do_chop      );
DECLARE_DO_FUN(do_bite      );
DECLARE_DO_FUN(do_melee     );
DECLARE_DO_FUN(do_brawl     );
DECLARE_DO_FUN(do_guard     );
DECLARE_DO_FUN(do_leg_sweep );
DECLARE_DO_FUN(do_uppercut  );
DECLARE_DO_FUN(do_second    );
DECLARE_DO_FUN(do_war_cry   ); 
DECLARE_DO_FUN(do_tumble    );
DECLARE_DO_FUN(do_distract  );
DECLARE_DO_FUN(do_feint     );
DECLARE_DO_FUN(do_net       );
DECLARE_DO_FUN(do_headbutt  );
DECLARE_DO_FUN(do_aim       );
DECLARE_DO_FUN(do_semiauto  );
DECLARE_DO_FUN(do_fullauto  );
DECLARE_DO_FUN(do_hogtie    );
DECLARE_DO_FUN(do_snipe     );
DECLARE_DO_FUN(do_burst     );
DECLARE_DO_FUN(do_drunken_fury); 
DECLARE_DO_FUN(do_shield_bash);
DECLARE_DO_FUN(do_spit      );
DECLARE_DO_FUN(do_choke_hold);
DECLARE_DO_FUN(do_hurl      );
DECLARE_DO_FUN(do_roundhouse);
DECLARE_DO_FUN(do_look      );
DECLARE_DO_FUN(do_restore   );
DECLARE_DO_FUN(do_fatal_blow);
DECLARE_DO_FUN(do_stance    );
DECLARE_DO_FUN(do_fervent_rage    );
DECLARE_DO_FUN(do_paroxysm    );

DECLARE_SPELL_FUN( spell_windwar        );
DECLARE_SPELL_FUN( spell_lightning_bolt );
DECLARE_SPELL_FUN( spell_call_lightning );
DECLARE_SPELL_FUN( spell_monsoon        );
DECLARE_SPELL_FUN( spell_confusion      );
DECLARE_SPELL_FUN( spell_laughing_fit   );

DECLARE_SPEC_FUN(   spec_executioner    );
DECLARE_SPEC_FUN(   spec_guard          );

/*
* Local functions.
*/
bool check_critical  args( ( CHAR_DATA *ch, bool secondary ) );
bool check_kill_trigger( CHAR_DATA *ch, CHAR_DATA *victim );
bool stop_attack( CHAR_DATA *ch, CHAR_DATA *victim );
bool check_outmaneuver( CHAR_DATA *ch, CHAR_DATA *victim );
bool check_mirror( CHAR_DATA *ch, CHAR_DATA *victim, bool show );
bool check_phantasmal( CHAR_DATA *ch, CHAR_DATA *victim, bool show );
bool check_fade( CHAR_DATA *ch, CHAR_DATA *victim, bool show );
bool check_avoid_hit( CHAR_DATA *ch, CHAR_DATA *victim, bool show );
bool blind_penalty( CHAR_DATA *ch );
void  check_assist  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool  check_dodge   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void  check_killer  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool  check_parry   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool  check_shield_block  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void  dam_message   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
						 int dt, bool immune ) );
void  death_cry     args( ( CHAR_DATA *ch ) );
void  group_gain    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int   xp_compute    args( ( CHAR_DATA *gch, CHAR_DATA *victim, int gain_align ) );
bool  is_safe       args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void  make_corpse   args( ( CHAR_DATA *ch, CHAR_DATA *killer, bool to_morgue ) );
void  split_attack  args( ( CHAR_DATA *ch, int dt ) );
void  one_hit       args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ));
void  mob_hit       args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void  raw_kill      args( ( CHAR_DATA *victim, CHAR_DATA *killer, bool to_morgue ) );
void  set_fighting  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool  disarm        args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool quiet ) );
void  behead        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool  check_duck    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool  check_jam     args( ( CHAR_DATA *ch, int odds, bool both ) );
void  check_stance  args( ( CHAR_DATA *ch ) );
void  warfare       args( ( char *argument ) );
void  add_war_kills args( ( CHAR_DATA *ch ) );
void  war_end       args( ( bool success ) );
bool  check_lose_stance args( (CHAR_DATA *ch) );
void  special_affect_update args( (CHAR_DATA *ch) );
void  death_penalty  args( ( CHAR_DATA *ch ) );
bool  check_mercy args( ( CHAR_DATA *ch ) );
void  check_reset_stance args( ( CHAR_DATA *ch) );
void  stance_hit    args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool check_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam_type, int skill );
bool is_normal_hit( int dt );
bool full_dam( CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,
             bool show );
bool is_safe_check( CHAR_DATA *ch, CHAR_DATA *victim,
                    bool area, bool quiet, bool theory );
bool check_kill_steal( CHAR_DATA *ch, CHAR_DATA *victim );


bool check_critical(CHAR_DATA *ch, bool secondary)
{
    OBJ_DATA *obj;
    
    if (!secondary)
        obj = get_eq_char( ch, WEAR_WIELD );
    else
        obj = get_eq_char( ch, WEAR_SECONDARY );

    if ( obj == NULL 
	|| get_skill(ch,gsn_critical) <  1 
	|| get_weapon_skill( ch, get_weapon_sn_new( ch, secondary ) ) < 95 
	|| number_range(0,100) > get_skill(ch,gsn_critical) )
                  return FALSE;
        
	/* 3% chance to work */
     int roll=number_percent();
     if ( roll < 98 )
        return FALSE;
	
     else	
        return TRUE;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    bool reverse_order;

    reverse_order = (number_bits(1) == 0);

    /* reverse the order in which the chars attack */
    if ( reverse_order )
	reverse_char_list();
    
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

        check_reset_stance(ch);

	/* handle affects that do things each round */
	special_affect_update(ch);
        
        /*
        * Hunting mobs. 
        */
        if (IS_NPC(ch) && ch->hunting && (ch->fighting == NULL) && IS_AWAKE(ch))
        {
	    hunt_victim(ch);
            continue;
        }
        
        if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
            continue;
        
	check_jump_up(ch);

        if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
            multi_hit( ch, victim, TYPE_UNDEFINED );
        else
            stop_fighting( ch, FALSE );
        
        if ( ( victim = ch->fighting ) == NULL )
            continue;
        
       /*
        * Fun for the whole family!
        */
        check_assist(ch,victim);
	check_rescue( ch );
        
        if ( IS_NPC( ch ) )
        {
            if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
                mp_percent_trigger( ch, victim, NULL, NULL, TRIG_FIGHT );
            if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
                mp_hprct_trigger( ch, victim );
            if ( HAS_TRIGGER( ch, TRIG_MPCNT ) )
                mp_mprct_trigger( ch, victim );
        }
    }
    
    /* restore the old order in char list */
    if ( reverse_order )
	reverse_char_list();
}

void reverse_char_list()
{
    CHAR_DATA 
        *new_char_list = NULL,
        *next_char;
    
    if ( char_list == NULL || char_list->next == NULL)
        return;
    
    next_char = char_list->next;
    while ( next_char != NULL )
    {
        char_list->next = new_char_list;
        new_char_list = char_list;
        char_list = next_char;
        next_char = char_list->next;
    }
    char_list->next = new_char_list;
}

/* execute a default combat action */
void run_combat_action( DESCRIPTOR_DATA *d )
{
    char *command;
    CHAR_DATA *ch;

    if ( d == NULL
	 || (ch = d->character) == NULL
	 || ch->pcdata == NULL
	 || (command = ch->pcdata->combat_action) == NULL
	 || command[0] == '\0' )
	return;

    /* no actions for charmed chars */
    if ( IS_AFFECTED(ch, AFF_CHARM) )
	return;

    /*
    if ( !run_olc_editor( d ) )
	substitute_alias( d, command );
    else
	nanny( d, command );
    */
    anti_spam_interpret( d->character, command );

    /* prevent spam from lag-less actions */
    if ( ch->wait == 0 )
	WAIT_STATE( ch, PULSE_VIOLENCE );
}

bool wants_to_rescue( CHAR_DATA *ch )
{
  return (IS_NPC(ch) && IS_SET(ch->off_flags, OFF_RESCUE))
      || PLR_ACT(ch, PLR_AUTORESCUE);
}

/* check if character rescues someone */
void check_rescue( CHAR_DATA *ch )
{
  CHAR_DATA *other, *target;
  char buf[MSL];

  if ( !wants_to_rescue(ch) )
    return;

  /* get target */
  if ( !IS_NPC(ch) || ch->leader != NULL )
  {
      bool need_rescue = FALSE;

      target = ch->leader;
      if ( target == NULL )
	  return;

      /* check if anyone fights the target */
      for ( other=ch->in_room->people; other != NULL; other=other->next_in_room )
	  if ( other->fighting == target && !is_safe_spell(ch, other, FALSE))
	  {
	      need_rescue = TRUE;
	      break;
	  }
      if (!need_rescue)
	  return;
  }
  else
  {
      target = NULL;
      for ( other=ch->in_room->people; other != NULL; other=other->next_in_room )
	  if ( !is_same_group(ch, other)
	       && other->fighting != NULL
	       && other->fighting != ch
	       && is_same_group(ch, other->fighting)
	       && can_see(ch, other->fighting) )
	  {
	      target = other->fighting;
	      break;
	  }
  }

  if (target == NULL || target == ch)
    return;

  if (ch->position <= POS_SLEEPING || !can_see(ch, target))
    return;

  /* lag-free rescue */
  if (number_percent() < get_skill(ch, gsn_bodyguard))
  {
    int old_wait = ch->wait;
    sprintf(buf, "You rush in to protect %s.\n\r", target->name );
    send_to_char( buf, ch );
    do_rescue( ch, target->name );
    ch->wait = old_wait;
    check_improve(ch, gsn_bodyguard, TRUE, 3);
    return;
  }

  /* normal rescue - wait check */
  if (ch->wait > 0 || (ch->desc != NULL && is_command_pending(ch->desc)))
    return;
  
  sprintf(buf, "You try to protect %s.\n\r", target->name );
  send_to_char( buf, ch );
  do_rescue( ch, target->name );
}

/* saving throw against attacks that affect the body */
bool save_body_affect( CHAR_DATA *ch, int level )
{
    int resist = ch->level * (100 + get_curr_stat(ch, STAT_VIT)) / 200;
    return number_range( 0, level ) < number_range( 0, resist );
}

/* handle affects that do things each round */
void special_affect_update(CHAR_DATA *ch)
{
    AFFECT_DATA *af;
	
    /* guard */
    /*
    if (is_affected(ch, gsn_guard) && ch->fighting == NULL)
    {
	affect_strip(ch, gsn_guard);
	if ( skill_table[gsn_guard].msg_off )
	{
	    send_to_char( skill_table[gsn_guard].msg_off, ch );
	    send_to_char( "\n\r", ch );
	}
    }
    */

    /* choke hold */
    if (is_affected(ch, gsn_choke_hold))
    {
	int choke_level = 0;
	int chance, dam;
	
	/* determine strength of choking affect */
	for ( af = ch->affected; af != NULL; af = af->next )
	    if ( af->type == gsn_choke_hold )
	    {
		choke_level = af->level;
		break;
	    }
	
	/* try to break free of choke hold */
	chance = 100 + (get_curr_stat(ch, STAT_STR) - choke_level) / 8;
	if ( number_percent() <= chance/6 || ch->fighting == NULL)
	{
	    if (ch->fighting != NULL)
	    {
		send_to_char("You struggle out of the choke hold.\n\r", ch);
		act( "$n struggles free and gasps for air.",
		     ch, NULL, NULL, TO_ROOM );
	    }
	    affect_strip(ch, gsn_choke_hold);
	    if ( skill_table[gsn_choke_hold].msg_off )
	    {
		send_to_char( skill_table[gsn_choke_hold].msg_off, ch );
		send_to_char( "\n\r", ch );
	    }
	}
	else
	{
	    chance = 50 + (get_curr_stat(ch, STAT_VIT) - choke_level) / 8;
	    send_to_char("You choke towards a slow death!\n\r", ch);
	    dam = UMAX(ch->level, 10);
	    /* resist roll to half damage */
	    if ( ch->move > 0 && number_percent() <= chance )
		dam /= 2;
	    ch->move = UMAX(0, ch->move - dam);
	    /* resist roll to avoid hp loss */
	    if ( (ch->move == 0) /* no more air :) */
		 || number_percent() > chance )
		ch->hit = UMAX(1, ch->hit - dam);
	}
    }

    /* vampire sunburn */
    if ( IS_SET(ch->form, FORM_SUNBURN)
	 && (!IS_NPC(ch) || ch->pIndexData->vnum == MOB_VNUM_VAMPIRE)
	 && !IS_AFFECTED(ch, AFF_SHELTER)
	 // no linkdeads unless fighting
	 && (IS_NPC(ch) || ch->desc != NULL || ch->fighting != NULL)
	 && room_is_sunlit(ch->in_room) )
    {
	int sunlight;
	switch ( weather_info.sky )
	{
	case SKY_CLOUDLESS: sunlight = 200; break;
	case SKY_CLOUDY: sunlight = 150; break;
	case SKY_RAINING: sunlight = 100; break;
	case SKY_LIGHTNING: sunlight = 50; break;
	default: sunlight = 100; break;
	}
	if ( weather_info.sunlight == SUN_LIGHT )
	    sunlight /= 2;

	/* tune to fit char level */
	sunlight = (ch->level + 10) * sunlight/100;
        
        if ( IS_AFFECTED(ch, AFF_SHROUD))
        {
            sunlight /= 2;
            send_to_char( "Your shroud absorbs part of the sunlight.\n\r", ch );
	    full_dam( ch, ch, sunlight, gsn_torch, DAM_LIGHT, TRUE );
        }
	else if ( !saves_spell(sunlight, ch, DAM_LIGHT) )
	{
	    send_to_char( "The sunlight burns your skin!\n\r", ch );
	    /* misuse torch skill damage message */
	    full_dam( ch, ch, sunlight, gsn_torch, DAM_LIGHT, TRUE );
	}
    }

    /* shan-ya battle madness */
    if ( ch->fighting != NULL && !IS_AFFECTED(ch, AFF_BERSERK)
	 && number_bits(5) == 0 && check_skill(ch, gsn_shan_ya) )
    {
	AFFECT_DATA af;

        af.where    = TO_AFFECTS;
        af.type     = gsn_shan_ya;
        af.level    = ch->level;
        af.duration = 1;
        af.modifier = 2 * (10 + ch->level);
        af.bitvector = AFF_BERSERK;
        
        af.location = APPLY_HITROLL;
        affect_to_char(ch,&af);

        af.location = APPLY_DAMROLL;
        affect_to_char(ch,&af);
 	
        send_to_char( "{WYou're enraged with shan-ya battle madness!{x\n\r", ch);
        act( "{W$n is enraged with shan-ya battle madness!{x", ch,NULL,NULL,TO_ROOM);
    }

    /* divine healing */
    if ( ch->hit < ch->max_hit && IS_AFFECTED(ch, AFF_HEAL) )
    {
	int heal;

	if ( ch->level < 90 || IS_NPC(ch) )
	    heal = 10 + ch->level;
	else
	    heal = 100 + 10 * (ch->level - 90);

	send_to_char( "Your wounds mend.\n\r", ch ); 
	ch->hit = UMIN(ch->max_hit, ch->hit + heal);
	update_pos( ch );
    }

    /* replenish healing */
    if ( ch->hit < ch->max_hit && IS_AFFECTED(ch, AFF_REPLENISH) )
    {
	int heal;

	if ( ch->level < 90 || IS_NPC(ch) )
	    heal = 20 + ch->level;
	else
	    heal = 150 + 10 * (ch->level - 90);

	send_to_char( "You replenish yourself.\n\r", ch ); 
	ch->hit = UMIN(ch->max_hit, ch->hit + heal);
	update_pos( ch );
    }

    /* Infectious Arrow - DOT - Damage over time */
    if ( is_affected(ch, gsn_infectious_arrow) )
    {

	int infect;
        infect = number_range (6,30);
        infect += ch->level/6;

        send_to_char( "Your festering wound oozes blood.\n\r", ch );
        full_dam( ch, ch, infect, gsn_infectious_arrow, DAM_DISEASE, TRUE );
	update_pos( ch );
    }


    /* Rupture - DOT - Damage over time */
    if ( is_affected(ch, gsn_rupture) )
    {
	int infect;

        infect = number_range (5, 25);
        infect += ch->level/5;

	send_to_char( "Your ruptured wound oozes blood.\n\r", ch ); 
        full_dam( ch, ch, infect, gsn_rupture, DAM_PIERCE, TRUE );
	update_pos( ch );
    }

    /* Paralysis - DOT - Damage over time - Astark Oct 2012 */
    if ( IS_AFFECTED(ch, AFF_PARALYSIS) )
    {
	int infect;

        infect = number_range (20, 40);
        infect += ch->level/5;

	send_to_char( "The paralyzing poison cripples you.\n\r", ch ); 
        full_dam( ch, ch, infect, gsn_paralysis_poison, DAM_POISON, TRUE );
	update_pos( ch );
    }
    
}

/* check wether a char succumbs to his fears and tries to flee */
bool check_fear( CHAR_DATA *ch )
{
    if ( ch == NULL )
	return FALSE;

    if ( !IS_AFFECTED(ch, AFF_FEAR)
	 || ch->fighting == NULL
	 || number_bits(1) == 0 )
	return FALSE;
    
    send_to_char( "Overwelmed by your fears to try to flee!\n\r", ch );
    do_flee( ch, "" );
    return TRUE;
}

/* checks whether to reset a mob to its standard stance
 */
void check_reset_stance(CHAR_DATA *ch)
{
    int chance;
    
    if ( !IS_NPC(ch) || ch->stance != STANCE_DEFAULT
	 || ch->pIndexData->stance == STANCE_DEFAULT
	 || ch->position < POS_FIGHTING )
      return;
    
    if ( is_affected(ch, gsn_paroxysm) )
    {
        ch->stance = 0;
        return;
    }

    chance = 20 + ch->level / 4;
    if (number_percent() < chance)
        do_stance(ch, stances[ch->pIndexData->stance].name);
}


/* checks whether to jump up during combat
 */
void check_jump_up( CHAR_DATA *ch )
{
    int chance;
    
    if ( ch == NULL || ch->fighting == NULL
	 || ch->position >= POS_FIGHTING
	 || ch->position <= POS_SLEEPING )
      return;

    chance = 25 + ch->level/4 + get_curr_stat(ch, STAT_AGI)/8
	+ get_skill(ch, gsn_jump_up) / 4;
    if ( ch->daze > 0 )
        chance /= 2;

    if ( is_affected(ch, gsn_hogtie) )
        chance *= 2/3;

    if (number_percent() < chance)
    {
        act( "You leap to your feet!", ch, NULL, NULL, TO_CHAR);
        act( "$n leaps to $s feet!", ch, NULL, NULL, TO_ROOM);
	set_pos( ch, POS_FIGHTING );
	check_improve( ch, gsn_jump_up, TRUE, 3);
    }
    else
	check_improve( ch, gsn_jump_up, FALSE, 3);
}


/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
        rch_next = rch->next_in_room;
        
        if (IS_AWAKE(rch) && rch->fighting == NULL)
        {
            
            /* quick check for ASSIST_PLAYER */
            if (!IS_NPC(ch) && IS_NPC(rch) && IS_NPC(victim)
		&& rch != victim
                && IS_SET(rch->off_flags,ASSIST_PLAYERS)
                && rch->level + 6 > victim->level)
            {
                do_emote(rch,"screams and attacks!");
                multi_hit(rch,victim,TYPE_UNDEFINED);
                continue;
            }
            
            /* PCs next */
            if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
            {
                if ( ((!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		      || IS_AFFECTED(rch,AFF_CHARM)) 
		     && is_same_group(ch,rch) 
		     && !is_safe(rch, victim))
                    multi_hit (rch,victim,TYPE_UNDEFINED);
                
                continue;
            }
            
            /* now check the NPC cases */
            
            if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM) && IS_NPC(rch))
            {
                if ( (IS_SET(rch->off_flags,ASSIST_ALL))
		     || (rch->group && rch->group == ch->group)
		     || (rch->race == ch->race && IS_SET(rch->off_flags,ASSIST_RACE))
		     || (IS_SET(rch->off_flags,ASSIST_ALIGN)
			 && ((IS_GOOD(rch) && IS_GOOD(ch))
			     || (IS_EVIL(rch) && IS_EVIL(ch))
			     || (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))
		     || (IS_SET(rch->off_flags, ASSIST_GUARD)
			 && (ch->spec_fun == spec_guard
			     || ch->spec_fun == spec_executioner))
		     || (rch->pIndexData == ch->pIndexData 
			 && IS_SET(rch->off_flags,ASSIST_VNUM)))
                    
                {
                    CHAR_DATA *vch;
                    CHAR_DATA *target;
                    int number;
                    
		    /*
                    if (number_bits(1) == 0)
                        continue;
		    */
                    
                    target = NULL;
                    number = 0;
                    for (vch = ch->in_room->people; vch; vch = vch->next)
                    {
                        if (can_see(rch,vch)
                            && is_same_group(vch,victim)
			    && !is_safe(rch, vch)
                            && number_range(0,number) == 0)
                        {
                            target = vch;
                            number++;
                        }
                    }
                    
                    if (target != NULL)
                    {
                        do_emote(rch,"screams and attacks!");
                        multi_hit(rch,target,TYPE_UNDEFINED);
			continue;
                    }
                }   
            }
        }
    }
}


/* performs combat actions due to stances for PCs and NPCs alike
 */
void stance_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int i, tempest;
    
    if (ch->stance==STANCE_JIHAD
	|| ch->stance==STANCE_KAMIKAZE
	|| ch->stance==STANCE_GOBLINCLEAVER)
	for (i=0; i<3; i++)
	{
	    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	    {
		vch_next = vch->next_in_room;
		if (vch->fighting && (is_same_group(vch->fighting, ch)))
		    one_hit(ch,vch,dt, FALSE);
	    }
	    /* check if more attacks follow.. */
	    if ( ch->stance == STANCE_GOBLINCLEAVER )
		continue;
	    if ( ch->stance == STANCE_KAMIKAZE
		 && ch->hit <= ch->max_hit/4 
		 && number_percent() < get_skill(ch, gsn_ashura) )
		continue;
	    break;
	}
        
    /*
    if (ch->stance==STANCE_GOBLINCLEAVER)
	for (i=0; i<3; i++)
	    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
            {
		vch_next = vch->next_in_room;
		if (vch->fighting && is_same_group(vch->fighting, ch))
		    one_hit(ch,vch,dt, FALSE);
	    }
    */
      
    if ( ch->stance == STANCE_ANKLEBITER )
    {
	int wait = ch->wait;
	bool found = FALSE;
	switch ( number_range(0, 3) )
	{
	case 0:
	    if ( get_skill(ch, gsn_trip) > 0 
		 && (!IS_AFFECTED(victim,AFF_FLYING) || IS_AFFECTED(ch,AFF_FLYING))
		 && victim->position >= POS_FIGHTING )
	    {
		do_trip( ch, "" );
		found = TRUE;
	    }
	    break;
	case 1:
	    if ( get_skill(ch, gsn_disarm) > 0 
		 && can_see(ch, victim)
		 && get_eq_char(victim, WEAR_WIELD) )
	    {
		do_disarm( ch, "" );
		found = TRUE;
	    }
	    break;
	case 2:
	    if ( get_skill(ch, gsn_mug) > 0 )
	    {
		do_mug( ch, "" );
		found = TRUE;
	    }
	    break;
	case 3:
	    if ( get_skill(ch, gsn_dirt) > 0
		 && !IS_AFFECTED(victim, AFF_BLIND)
		 && ch->in_room->sector_type != SECT_WATER_DEEP 
		 && ch->in_room->sector_type != SECT_WATER_SHALLOW 
		 && ch->in_room->sector_type != SECT_AIR )
	    {
		do_dirt( ch, "" );
		found = TRUE;
	    }
	    break;
	}
	if ( !found && get_skill(ch, gsn_bite) > 0 )
	    do_bite( ch, "" );
	ch->wait = wait;
    }

    if (ch->stance==STANCE_WENDIGO)
        if ( get_eq_char( victim, WEAR_WIELD ) != NULL )
            if ( number_percent() < 15 )   disarm(ch,victim,FALSE);
                   
    if (ch->stance == STANCE_TEMPEST)
	if (ch->mana > 50)
	{
	    int skill;
	    if ( !IS_OUTSIDE(ch) )
		tempest = 2;
	    else
	    {
		if (weather_info.sky < SKY_RAINING)
		    tempest = number_range (1, 2);
		else
		    tempest = number_range (1, 4);
	    }
	    switch (tempest) 
	    {
	    case(1):
		skill=skill_lookup("wind war");
		if (get_skill(ch,skill)>number_percent())
		    {
			ch->mana-=skill_table[skill].min_mana/2;
			spell_windwar( skill, ch->level,ch,victim,TARGET_CHAR);
		    }
		break;
	    case(2):
		skill=skill_lookup("lightning bolt");
		if (get_skill(ch,skill)>number_percent())
		    {
			ch->mana-=skill_table[skill].min_mana/2;
			spell_lightning_bolt( skill, ch->level,ch,victim,TARGET_CHAR);
		    }
		break;
	    case(3):
		skill=skill_lookup("call lightning");
		if (get_skill(ch,skill)>number_percent())
		    {
			ch->mana-=skill_table[skill].min_mana/2;
			spell_call_lightning( skill, ch->level,ch,victim,TARGET_CHAR);
		    }
		break;
	    case(4):
		skill=skill_lookup("monsoon");
		if (get_skill(ch,skill)>number_percent())
		    {
			ch->mana-=skill_table[skill].min_mana/2;
			spell_monsoon( skill, ch->level,ch,victim,TARGET_CHAR);
		    }
		break;
	    }
	}
	else
	{
	    send_to_char("The tempest fizzles as you run out of energy.\n\r",ch);
	    ch->stance=0;
	}
    
    CHECK_RETURN(ch, victim);
    if ( ch->stance!=0 && 
	 (ch->stance==STANCE_LION 
	  || ch->stance==STANCE_TIGER
	  || ch->stance==STANCE_WENDIGO
	  || ch->stance==STANCE_BLADE_DANCE
	  || ch->stance==STANCE_AMBUSH
	  || ch->stance==STANCE_RETRIBUTION
	  || ch->stance==STANCE_PORCUPINE
	  || (ch->stance==STANCE_TEMPEST 
	      && ch->in_room
	      && ch->in_room->sector_type >= SECT_WATER_SHALLOW
	      && ch->in_room->sector_type<=SECT_AIR)) )
        one_hit(ch, victim, dt, FALSE);

    CHECK_RETURN(ch, victim);
    if ( ch->stance==STANCE_TIGER
	 && get_eq_char(ch, WEAR_HOLD) == NULL
	 && get_eq_char(ch, WEAR_SHIELD) == NULL
	 && number_bits(1) )
        one_hit(ch, victim, dt, TRUE);

    CHECK_RETURN(ch, victim);
    if ( victim->stance==STANCE_KAMIKAZE )
        one_hit(ch, victim, dt, FALSE);
}


/*
* Do one group of attacks.
*/
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance, tempest;
    OBJ_DATA *wield;
    OBJ_DATA *second;

    /* decrement the wait */
    if (ch->desc == NULL)
        ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);
    
    if (ch->desc == NULL)
        ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 

    if (ch->stop>0)
    {
	ch->stop--;
	return;
    }
    
    /* safety-net */
    CHECK_RETURN(ch, victim);

    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
        return;
    
    if (IS_NPC(ch))
    {
        mob_hit(ch,victim,dt);
        return;
    }
    
    wield = get_eq_char( ch, WEAR_WIELD );
    
    check_stance(ch);
    check_killer( ch, victim );
    
    /* automatic attacks for brawl & melee */
    if (wield == NULL)
	chance = get_skill(ch, gsn_brawl);
    else
    {
	chance = get_skill(ch, gsn_melee) * 3/4;
	if ( wield->value[0] == WEAPON_POLEARM )
	    chance += 25;
    }

    if (number_percent() <= chance)
    {
      /* For each opponent beyond the first there's an extra attack */
      CHAR_DATA *vch, *vch_next;
      bool found = FALSE;
      for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
      {
	  vch_next = vch->next_in_room;
	  if ( vch->fighting != NULL
	       && is_same_group(vch->fighting, ch)
	       && ch->fighting != vch )
	  {
	      one_hit(ch, vch, dt, FALSE);
	      found = TRUE;
	  }
      }
      /* improve skill */
      if (found)
	  if (wield == NULL)
	      check_improve(ch, gsn_brawl, TRUE, 3);
	  else
	      check_improve(ch, gsn_melee, TRUE, 3);
    }

    /*
    if (ch->stance == STANCE_KAMIKAZE)
    {
        damage(ch,ch,ch->level/2+10,gsn_kamikaze,DAM_MENTAL,FALSE);
	if (ch->stance != STANCE_KAMIKAZE)
	    return;
    }
    */
    
    stance_hit(ch, victim, dt);
    
    if  (  (!IS_AFFECTED(ch,AFF_GUARD) || number_range(0,1))
         && (ch->stance!=STANCE_FIREWITCHS_SEANCE || number_range(0,1))
         && ch->stance!=STANCE_TORTOISE 
         && ch->stance!=STANCE_AVERSION )
        one_hit( ch, victim, dt, FALSE);
    else
	damage( ch, victim, 0, dt, DAM_BASH, FALSE );
                    
    if (ch->fighting != victim)
	return;
                    
    if (number_percent() < get_skill(ch, gsn_extra_attack) * 2/3 )
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    }

    if ( wield != NULL && wield->value[0] == WEAPON_DAGGER
	 && number_bits(4) == 0 )
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    }
    
    if ((second=get_eq_char (ch, WEAR_SECONDARY))!=NULL&&wield!=NULL)
    {
        chance = get_skill(ch,gsn_dual_wield);
        if ((wield->value[0]==WEAPON_DAGGER)&&(second->value[0]==WEAPON_DAGGER))
        {
            chance=UMAX(chance, tempest=get_skill(ch,gsn_dual_dagger));
            check_improve(ch,gsn_dual_dagger,TRUE,8);
        }
        else if ((wield->value[0]==WEAPON_SWORD)&&(second->value[0]==WEAPON_SWORD))
        {
            chance=UMAX(chance, tempest=get_skill(ch,gsn_dual_sword));
            check_improve(ch,gsn_dual_sword,TRUE,8);
        }
        else if ((wield->value[0]==WEAPON_AXE)&&(second->value[0]==WEAPON_AXE))
        {
            chance=UMAX(chance, tempest=get_skill(ch,gsn_dual_axe));
            check_improve(ch,gsn_dual_axe,TRUE,8);
        }
        else if ((wield->value[0]==WEAPON_GUN)&&(second->value[0]==WEAPON_GUN))
        {
            chance=UMAX(chance, tempest=get_skill(ch,gsn_dual_gun));
            check_improve(ch,gsn_dual_gun,TRUE,8);
        }
        check_improve(ch,gsn_dual_wield,TRUE,10);
        
        chance += ch_dex_extrahit(ch);
        
        if (get_eq_char(ch, WEAR_SHIELD))
        {
	    chance = chance * (100 + get_skill(ch, gsn_wrist_shield)) / 300;
            check_improve(ch, gsn_wrist_shield, TRUE, 20);
        }
        
        if (number_percent() < chance)
        {
            one_hit( ch, victim, dt, TRUE );
            if ( ch->fighting != victim )
                return;
            if (number_percent() < get_skill(ch, gsn_extra_attack) / 3)
            {
                one_hit(ch,victim,dt,TRUE);
                if (ch->fighting != victim) return;
            }
        }
        
        if (IS_AFFECTED(ch,AFF_HASTE))
        {
            chance -=50;
            if (number_percent() <chance)
            {
                one_hit(ch,victim,dt,TRUE);
                if (ch->fighting != victim)
                    return;
            }     
        }

	if ( second->value[0] == WEAPON_DAGGER
	     && number_bits(4) == 0 )
	{
	    one_hit(ch,victim,dt,TRUE);
	    if (ch->fighting != victim)
		return;
	}

    } // second != NULL

    if (wield == NULL && second == NULL &&
        get_eq_char(ch, WEAR_HOLD) == NULL && 
        get_eq_char(ch, WEAR_SHIELD) == NULL)
    {
        chance = ch->level + ch_dex_extrahit(ch);
        if (number_percent() < chance)
            one_hit(ch, victim, dt, TRUE);
        if (ch->fighting != victim)
            return;
        if (IS_AFFECTED(ch,AFF_HASTE))
        {
            chance -= 50;
            if (number_percent() < chance)
            {
                one_hit(ch, victim, dt, TRUE);
                if (ch->fighting != victim)
                    return;
            }
        }
	if (number_percent() < get_skill(ch, gsn_extra_attack) / 3)
        {
	    one_hit(ch,victim,dt,TRUE);
	    if (ch->fighting != victim)
		return;
	}
    }

    if ( IS_AFFECTED(ch, AFF_HASTE) )
    {
        one_hit(ch,victim,dt,FALSE);
        if (ch->fighting != victim)
            return;
    }

    if ( IS_AFFECTED(ch, AFF_MANTRA) && ch->mana > 0 )
    {
        ch->mana -= 1;
        one_hit(ch,victim,dt,FALSE);
        if (ch->fighting != victim)
            return;
    }
    
    
    if ( ch->fighting != victim || dt == gsn_backstab || dt == gsn_snipe)
        return;
    
    if (wield == NULL
	&& number_percent() < get_skill (ch, gsn_kung_fu) )
    {
        chance=ch->wait;
        do_chop(ch, "");
        if (ch->fighting != NULL)
            do_kick(ch, "");
        ch->wait = chance;
        check_improve(ch,gsn_kung_fu,TRUE,3);
	if ( ch->fighting != victim )
	    return;
    }
    
    if (number_percent() < get_skill(ch, gsn_maul))
    {
        chance = ch->wait;
        do_bite(ch, "");
        ch->wait = chance;
	if ( ch->fighting != victim )
	    return;
    }
    
    chance = get_skill(ch,gsn_second_attack) * 2/3 +  ch_dex_extrahit(ch);
    
    if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 2;

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE);
        check_improve(ch,gsn_second_attack,TRUE,5);
        if ( ch->fighting != victim )
            return;
    }
    
    chance = get_skill(ch,gsn_third_attack) * 2/3 + ch_dex_extrahit(ch);

    if (IS_AFFECTED(ch,AFF_SLOW) )
        chance = 0;

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE);
        check_improve(ch,gsn_third_attack,TRUE,6);
        if ( ch->fighting != victim )
            return;
    }

    if ( ch->hit <= ch->max_hit/4 
	 && number_percent() < get_skill(ch, gsn_ashura) )
    {
        one_hit( ch, victim, dt, FALSE);
        check_improve(ch,gsn_ashura,TRUE,3);
        if ( ch->fighting != victim )
            return;
    }
     
    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int i,chance,number;
    CHAR_DATA *vch, *vch_next;
    
    if (ch->stop>0)
    {
	ch->stop--;
	return;
    }

    /* mobs must check their stances too */
    check_stance(ch);

    if (!IS_AFFECTED(ch, AFF_GUARD) || number_range(0, 1))
	one_hit(ch,victim,dt,FALSE);

    if (ch->fighting != victim)
	return;

    /* high level mobs get extra attacks */
    for ( i = 120; i < ch->level; i += 20 )
    {
	if ( number_bits(1) || number_range(1,20) > ch->level - i )
	    continue;

	one_hit(ch,victim,dt,FALSE);

	if (ch->fighting != victim)
	    return;
    }

    stance_hit(ch, victim, dt);
    
    /* Area attack -- BALLS nasty! */
    
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
        {
            vch_next = vch->next;
            if (((vch != victim) && vch->fighting == ch))
                one_hit(ch,vch,dt, FALSE);
        }
    }
    
    if ( IS_AFFECTED(ch,AFF_HASTE) )
        one_hit(ch,victim,dt,FALSE);

    if (ch->fighting != victim)
	return;

    chance = get_skill(ch,gsn_second_attack) * 2/3;
    if ( IS_AFFECTED(ch, AFF_SLOW) )
	chance /= 2;

    if (number_percent() < chance)
    {
        one_hit(ch,victim,dt, FALSE);
        if (ch->fighting != victim)
            return;
    }
    
    chance = get_skill(ch,gsn_third_attack) * 2/3;
    if ( IS_AFFECTED(ch, AFF_SLOW) )
	chance /= 2;
    
    if (number_percent() < chance)
    {
        one_hit(ch,victim,dt,FALSE);
        if (ch->fighting != victim)
            return;
    } 

    if ( get_eq_char(ch, WEAR_SECONDARY) )
    {
        one_hit(ch,victim,dt, TRUE);
        if (ch->fighting != victim)
            return;
    }	
    
    /* oh boy!  Fun stuff! */
    
    if (ch->wait > 0)
        return;
    
    number = number_range(0,2);
    
    if (number == 1 && IS_SET(ch->act,ACT_MAGE))
    {
        /*  { mob_cast_mage(ch,victim); return; } */ ;
    }
    
    if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
    {   
        /* { mob_cast_cleric(ch,victim); return; } */ ;
    }
    
    /* now for the skills */
    
    number = number_range(0,9);
    
    if ( ch->position >= POS_FIGHTING )
	switch(number) 
	{
	case (0) :
	    if (IS_SET(ch->off_flags,OFF_BASH))
		do_bash(ch,"");
	    break;
	    
	case (1) :
	    if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
		do_berserk(ch,"");
	    break;
	    
	case (2) :
	    if (IS_SET(ch->off_flags,OFF_DISARM) 
		|| (get_weapon_sn(ch) != gsn_hand_to_hand 
		    && (IS_SET(ch->act,ACT_WARRIOR)
			||  IS_SET(ch->act,ACT_THIEF))))
		do_disarm(ch,"");
	    break;
	    
	case (3) :
	    if (IS_SET(ch->off_flags,OFF_KICK))
		do_kick(ch,"");
	    break;
	    
	case (4) :
	    if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
		do_dirt(ch,"");
	    break;
	    
	case (5) :
	    if (IS_SET(ch->off_flags,OFF_TAIL))
	    {
		    /*do_tail(ch,"");*/
		do_leg_sweep(ch, "");
	    }
	    break; 
	case (6) :
	    if (IS_SET(ch->off_flags,OFF_TRIP))
		do_trip(ch,"");
	    break;
	    
	case (7) :
	    if (IS_SET(ch->off_flags,OFF_CRUSH))
		{
		    do_crush(ch,"");
		}
	    break;
	case (8) :
	case (9) :
	    if (IS_SET(ch->off_flags,OFF_CIRCLE))
	    {
		do_circle(ch,"");
	    }
	}
    
}

int get_weapon_damage( OBJ_DATA *wield )
{
    int weapon_dam;

    if ( wield == NULL )
	return 0;
    
    if ( wield->pIndexData->new_format )
	weapon_dam = dice( wield->value[1], wield->value[2] );
    else
	weapon_dam = number_range( wield->value[1], wield->value[2] );

    /* sharpness! */
    if ( IS_WEAPON_STAT(wield, WEAPON_SHARP) )
    {
	if ( number_bits(5) == 0 )
	    weapon_dam *= 2;
    }

    return weapon_dam;
}

int get_weapon_damtype( OBJ_DATA *wield )
{
    /* half base and half defined damage */
    return MIX_DAMAGE( weapon_base_damage[wield->value[0]],
		       attack_table[wield->value[3]].damage );
}

/* returns the damage ch deals with one hit */
int one_hit_damage( CHAR_DATA *ch, int dt, OBJ_DATA *wield)
{
    int dam;

    /* basic damage */
    if ( IS_NPC(ch) && ch->pIndexData->new_format )
	dam = dice(ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
    else
	dam = ch->level + dice( 1, 4 );

    /* weapon damage */
    if ( wield != NULL )
    {
	int weapon_dam;
	bool has_shield = get_eq_char(ch, WEAR_SHIELD) != NULL;

	weapon_dam = get_weapon_damage( wield );
	
	/* twohanded weapons */
	if ( wield->value[0] == WEAPON_BOW )
	{
	    if ( has_shield )
		weapon_dam *= 1;
	    else
		weapon_dam *= 3;
	}
	else if ( IS_WEAPON_STAT(wield, WEAPON_TWO_HANDS) )
	{
	    int two_hand_bonus = (100 + get_skill(ch, gsn_two_handed)) / 2;
	    int wrist_bonus = (100 + get_skill(ch, gsn_wrist_shield)) / 3;
	    check_improve(ch, gsn_two_handed, TRUE, 10);
	    if ( has_shield )
	    {
		check_improve(ch, gsn_wrist_shield, TRUE, 10); 
		two_hand_bonus = two_hand_bonus * wrist_bonus / 100;
	    }
	    weapon_dam += weapon_dam * two_hand_bonus / 100;
	}
	else
	{
	    /* bonus for free off-hand */
	    if ( !has_shield
		 && get_eq_char(ch, WEAR_SECONDARY) == NULL
		 && get_eq_char(ch, WEAR_HOLD) == NULL )
		weapon_dam += weapon_dam / 2;
	}
	
	dam += weapon_dam;
    }
    else
    {
	/* level 90+ bonus */
	if ( !IS_NPC(ch) && ch->level > (LEVEL_HERO - 10) )
	    dam += ch->level - (LEVEL_HERO - 10);
    }

    /* damage roll */
    dam += GET_DAMROLL(ch) / 4;

    /* enhanced damage */
    if ( wield != NULL && (wield->value[0] == WEAPON_GUN
			   || wield->value[0] == WEAPON_BOW) )
    {
      /* Added snipe and aim here so that they aren't checked until further down - Astark 1-3-13 */
	if ( dt != gsn_burst && dt != gsn_semiauto && dt != gsn_fullauto && dt != gsn_snipe
	     && dt != gsn_aim && !number_bits(3) && chance(get_skill(ch, gsn_sharp_shooting)) )
	{
	    dam *= 2;
	    check_improve (ch, gsn_sharp_shooting, TRUE, 5);
	}
    
       /* Added this little hack in to improve damage on aim and snipe. We should fully test
          it later and compare damage numbers to the old aeaea binary. Works for now - Astark 1-3-13 */
  
       /* Updated on 1-6-13 to check for sharp shooting skill percentage */
 
        if ( dt == gsn_snipe )
        {
            if (number_bits(1))
            {
                dam *= get_skill(ch, gsn_sharp_shooting) / 20;
                check_improve (ch, gsn_sharp_shooting, TRUE, 6);
            }
            else
            {
                dam *= get_skill(ch, gsn_sharp_shooting) / 25;
                check_improve (ch, gsn_sharp_shooting, TRUE, 6);
            }
        }

        if ( dt == gsn_aim )
        {
            if (number_bits(1))
            {
                dam *= get_skill(ch, gsn_sharp_shooting) / 33;
                check_improve (ch, gsn_sharp_shooting, TRUE, 8);
            }
            else
            {
                dam *= get_skill(ch, gsn_sharp_shooting) / 40;
                check_improve (ch, gsn_sharp_shooting, TRUE, 8);
            }
        }

    }
    else
    {
	dam += ch->level * get_skill(ch, gsn_enhanced_damage) / 300;
	check_improve (ch, gsn_enhanced_damage, TRUE, 10);
	dam += ch->level * get_skill(ch, gsn_brutal_damage) / 300;
	check_improve (ch, gsn_brutal_damage, TRUE, 10);
    }

    return number_range( dam * 2/3, dam );
}

int martial_damage( CHAR_DATA *ch, int sn, CHAR_DATA *victim)
{
    int dam = one_hit_damage( ch, sn, NULL );

    if ( sn == gsn_bite )
	if ( IS_SET(ch->parts, PART_FANGS) )
	    return dam;
	else
	    return dam * 3/4;

    if ( sn == gsn_razor_claws )
	if ( IS_SET(ch->parts, PART_CLAWS) )
	    return dam;
	else
	    return dam * 3/4;

    if ( chance(get_skill(ch, gsn_kung_fu)) )
	dam += dam / 7;

    if ( sn == gsn_chop && get_eq_char(ch, WEAR_WIELD) == NULL )
	return dam;
    else
	return dam * 3/4;
}

/* for NPCs who run out of arrows */
void equip_new_arrows( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ch == NULL || get_eq_char(ch, WEAR_HOLD) != NULL )
	return;

    obj = create_object( get_obj_index( OBJ_VNUM_ARROWS ), 0 );

    if ( obj == NULL )
	return;

    obj->value[0] = 100;
    obj_to_char( obj, ch );
    wear_obj( ch, obj, FALSE );
}

void handle_arrow_shot( CHAR_DATA *ch, CHAR_DATA *victim, bool hit )
{
    OBJ_DATA *obj, *arrows = get_eq_char( ch, WEAR_HOLD );
    int dam, dam_type;
    bool equip_arrows = FALSE;

    /* better safe than sorry */
    if ( arrows == NULL || arrows->item_type != ITEM_ARROWS )
    {
	bugf( "handle_arrow_shot: no arrows" );
	return;
    }

    if ( arrows->value[0] <= 0 )
    {
	bugf( "handle_arrow: %d arrows", arrows->value[0] );
	extract_obj( arrows );
	return;
    }

    /* save data */
    dam = arrows->value[1];
    dam_type = arrows->value[2];

    /* reduce nr of arrows */
    arrows->value[0] -= 1;
    if ( arrows->value[0] == 0 )
    {
	extract_obj( arrows );
	send_to_char( "{+YOU HAVE RUN OUT OF ARROWS!{ \n\r", ch );
	equip_arrows = TRUE;
    }

    /* extra damage */
    CHECK_RETURN( ch, victim );
    if ( hit && dam > 0 )
	full_dam( ch, victim, dam, gsn_enchant_arrow, dam_type, TRUE );

    if ( IS_DEAD(ch) )
	return;

    /* auto-equip new arrows */
    if ( equip_arrows )
	if ( IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) )
	{
	    /* create new packet of arrows :) */
	    equip_new_arrows( ch );
	}
	else
	{
	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
		if ( obj->wear_loc == WEAR_NONE
		     && obj->item_type == ITEM_ARROWS )
		{
		    wear_obj( ch, obj, FALSE );
		    break;
		}
	}

    /* counterstrike */
    CHECK_RETURN( ch, victim );
    obj = get_eq_char( victim, WEAR_WIELD );
    if ( obj != NULL && (obj->value[0] == WEAPON_BOW || obj->value[0] == WEAPON_GUN)
	 || victim->fighting != ch || IS_AFFECTED(victim, AFF_FLEE) )
	return;
    one_hit( victim, ch, TYPE_UNDEFINED, FALSE );
}

/*
* Hit one guy once.
*/
void one_hit ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )
{
    OBJ_DATA *wield;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
    bool result, arrow_used = FALSE;
    /* prevent attack chains through re-retributions */
    static bool is_retribute = FALSE;

    sn = -1;
    
    if ( (dt == TYPE_UNDEFINED || is_normal_hit(dt))
	 && IS_AFFECTED(ch, AFF_INSANE)
	 && number_bits(1)==0 )
    {
	/* find new random victim */
        int i = 0;
        CHAR_DATA *m;
        for( m = ch->in_room->people; m != NULL; m = m->next_in_room )
            if ( !is_safe_spell(ch, m, TRUE) && ch != m )
	    {
		if ( number_range(0, i++) == 0 )
		    victim = m;
            }
    }

    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
        return;
    
    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
        return;
    
    /*
     * Figure out the type of damage message.
     */
    if (!secondary)
        wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_SECONDARY );
    
    /* bows are special */
    if ( wield != NULL && wield->value[0] == WEAPON_BOW )
    {
	OBJ_DATA *arrows = get_eq_char( ch, WEAR_HOLD );

	if ( arrows == NULL || arrows->item_type != ITEM_ARROWS )
	{
	    if ( !IS_NPC(ch) )
	    {
		send_to_char( "You must hold arrows to fire your bow.\n\r", ch );
		return;
	    }
	    else
	    {
		act( "$n removes $p.", ch, wield, NULL, TO_ROOM );
		unequip_char( ch, wield );
	    }
	}
	else
	{
	    if ( arrows->value[0] <= 0 )
	    {
		bugf( "one_hit: %d arrows", arrows->value[0] );
		extract_obj( arrows );
		return;
	    }
	    /* update arrows later - need to keep data for extra damage */
	    arrow_used = TRUE;
	}
    }

    if ( dt == TYPE_UNDEFINED )
    {
        dt = TYPE_HIT;
        if ( wield != NULL && wield->item_type == ITEM_WEAPON )
            dt += wield->value[3];
        else
            dt += ch->dam_type;
    }
    
    if (ch->stance && !wield && stances[ch->stance].martial)
        dam_type = stances[ch->stance].type;
    else if (wield && wield->item_type == ITEM_WEAPON)
	dam_type = get_weapon_damtype( wield );
    else
        dam_type = attack_table[ch->dam_type].damage;
    
    if (dam_type == -1)
        dam_type = DAM_BASH;
    
    /*added in by Korinn 1-19-99*/
    if (ch->stance == STANCE_KORINNS_INSPIRATION ||
        ch->stance == STANCE_PARADEMIAS_BILE)
        dam_type = stances[ch->stance].type;  
    /*ends here -Korinn-*/  

    /* get the weapon skill */
    sn = get_weapon_sn_new(ch, secondary);
    skill = 50 + get_weapon_skill(ch, sn) / 2;
    
    check_killer( ch, victim );

    if ( !check_hit(ch, victim, dt, dam_type, skill) )
    {
	/* Miss. */
	if (wield != NULL)
	    if (IS_SET(wield->extra_flags, ITEM_JAMMED))
		dt = gsn_pistol_whip;
	damage( ch, victim, 0, dt, dam_type, TRUE );
	if ( arrow_used )
	    handle_arrow_shot( ch, victim, FALSE );
	tail_chain( );
	return;
    }
        
    if (sn != -1)
	check_improve( ch, sn, TRUE, 10 );

    /*
     * Hit.
     * Calc damage.
     */
    dam = one_hit_damage( ch, dt, wield );

    if (wield != NULL)
    {
	if (IS_SET(wield->extra_flags, ITEM_JAMMED))
	{
	    if ( !number_bits(2) && IS_NPC(ch) )
		do_unjam( ch, "" );

	    if ( IS_SET(wield->extra_flags, ITEM_JAMMED) )
	    {
		if( secondary )
		    send_to_char("{yYOUR OFFHAND WEAPON IS JAMMED!{x\n\r", ch);
		else
		    send_to_char("{yYOUR WEAPON IS JAMMED!{x\n\r", ch);
		dam = 1 + ch->level * get_skill(ch,gsn_pistol_whip) / 100;
		check_improve (ch, gsn_pistol_whip, TRUE, 4);
		dt = gsn_pistol_whip;
	    }
	}

	/* spears do extra damage against large opponents */
	if ( wield->value[0] == WEAPON_SPEAR
	     && victim->size > SIZE_MEDIUM )
	{
	    if ( number_percent() <= get_skill(ch, gsn_giantfeller) )
	    {
		dam += dam * (victim->size - SIZE_MEDIUM) / 10;
		check_improve( ch, gsn_giantfeller, 10, TRUE );
	    }
	    else
		dam += dam * (victim->size - SIZE_MEDIUM) / 20;
	}

    }
    else
    {
	if ( IS_SET(ch->parts, PART_CLAWS) && ch->stance == 0 )
	{
	    dam += dam / 10;
	}
    }

    if ( IS_AFFECTED(ch, AFF_WEAKEN) )
	dam = dam * 9/10;

    if ( check_critical(ch,secondary) == TRUE )
    {
	act("$p {RCRITICALLY STRIKES{x $n!",victim,wield,NULL,TO_NOTVICT);
        act("{RCRITICAL STRIKE!{x",ch,NULL,victim,TO_VICT);
        check_improve(ch,gsn_critical,TRUE,4);
        dam *= 2;
    }

 /* Temporary fix for backstab and circle to check anatomy for
    damage boost - Astark 1-6-13 */
    if ( dt == gsn_backstab || dt == gsn_circle )
    {
       dam += dam * get_skill(ch, gsn_anatomy) / 20;
       check_improve( ch, gsn_anatomy, TRUE, 8 );
    } 
    
    /* leadership and charisma of group leader */
    if ( ch->leader != NULL
	 && ch->leader != ch 
	 && ch->in_room == ch->leader->in_room )
    {
	int bonus = get_curr_stat( ch->leader, STAT_CHA ) - 50;
	bonus += get_skill( ch->leader, gsn_leadership );
	bonus += ch->leader->level - ch->level;
	dam += dam * bonus / 1000;
	if ( number_bits(4) == 0 )
	    check_improve( ch->leader, gsn_leadership, TRUE, 10 );
    }
    
    if ( dam <= 0 )
	dam = 1;
    
    result = full_dam( ch, victim, dam, dt, dam_type, TRUE );
    
    /* arrow damage & handling */
    if ( arrow_used )
	handle_arrow_shot( ch, victim, result );

    CHECK_RETURN( ch, victim );

    /* if not hit => no follow-up effects.. --Bobble */
    if ( !result )
	return;
    
    /* funky weapons */
    weapon_flag_hit( ch, victim, wield );
    CHECK_RETURN( ch, victim );

    /* behead */
    check_behead( ch, victim, wield );
    CHECK_RETURN( ch, victim );

    /* aura */
    aura_damage( ch, victim, wield, dam );
    CHECK_RETURN( ch, victim );
    
    /* stance effects */
    stance_after_hit( ch, victim, wield );
    CHECK_RETURN( ch, victim );

    /* retribution */
    if ( (victim->stance == STANCE_PORCUPINE 
	  || victim->stance == STANCE_RETRIBUTION)
	 && !is_retribute
	 && !IS_AFFECTED(victim, AFF_FLEE) )
    {
	is_retribute = TRUE;
	one_hit(victim, ch, TYPE_UNDEFINED, FALSE);
	is_retribute = FALSE;
    }
    
   tail_chain( );
   return;
}

bool check_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam_type, int skill )
{
    int ch_roll, victim_roll;
    int attack_factor, defense_factor, victim_ac;
    int ac_dam_type;
    OBJ_DATA *wield;

    if ( ch == victim )
	return TRUE;

    /* special skill adjustment */
    if ( dt == gsn_aim
	 || dt == gsn_backstab
	 || dt == gsn_back_leap
	 || dt == gsn_circle
	 || dt == gsn_slash_throat
         || dt == gsn_rupture
	 || dt == gsn_snipe )
    {
	//if ( chance(get_skill(ch, dt)) )
	return TRUE;
    }
    
    if ( IS_AFFECTED(ch, AFF_CURSE) && per_chance(5) )
	return FALSE;

    if ( dt == gsn_fullauto
	 || dt == gsn_semiauto
	 || dt == gsn_burst )
    {
	int skill = get_skill(ch, dt);
	if ( !chance(50 + skill/4) )
	    return FALSE;
    }

    
    /* size */
    if ( number_percent() <= 3 * (SIZE_GIANT - victim->size) )
	return FALSE;

    if ( number_percent() <= get_skill(ch, gsn_giantfeller)
	 && number_percent() <= 3 * (victim->size - ch->size) )
    {
	check_improve(ch, gsn_giantfeller, TRUE, 6); 
	return TRUE;
    }

    /* automatic chance-to-hit */
    if ( number_bits(3) == 0 )
	return TRUE;

    /* ac */
    if ( IS_MIXED_DAMAGE(dam_type) )
	ac_dam_type = SECOND_DAMAGE( dam_type );
    else
	ac_dam_type = FIRST_DAMAGE( dam_type );

    switch( ac_dam_type )
    {
    case(DAM_PIERCE): victim_ac = GET_AC(victim,AC_PIERCE)/10;   break;
    case(DAM_BASH):   victim_ac = GET_AC(victim,AC_BASH)/10;     break;
    case(DAM_SLASH):  victim_ac = GET_AC(victim,AC_SLASH)/10;    break;
    default:          victim_ac = GET_AC(victim,AC_EXOTIC)/10;   break;
    }

    /* basic attack skill */
    if ( IS_NPC(ch) )
    {
	attack_factor = 100;
        if (IS_SET(ch->act,ACT_WARRIOR))
            attack_factor += 20;
        if (IS_SET(ch->act,ACT_MAGE))
            attack_factor -= 20;
    }
    else
    {
        attack_factor = class_table[ch->class].attack_factor;
    }

    /* basic defense skill */
    if ( IS_NPC(victim) )
    {
	defense_factor = 100;
        if (IS_SET(victim->act, ACT_WARRIOR))
            defense_factor += 20;
        if (IS_SET(victim->act, ACT_MAGE))
            defense_factor -= 20;
    }
    else
    {
        defense_factor = class_table[victim->class].defense_factor;
    }

    /* basic values */
    ch_roll = (ch->level + 10) * attack_factor/100 + GET_HITROLL(ch);
    victim_roll = (victim->level + 10) * defense_factor/200 + (10 - victim_ac);
    
    /* skill-based chance-to-miss */
    /*
    if ( number_percent() > skill )
	return FALSE;
    */
    ch_roll = ch_roll * skill/100;

    /* blind attacks */
    if ( !can_see( ch, victim ) && blind_penalty(ch) )
	ch_roll = ch_roll * 3/4;

    /* bad combat position */
    if ( ch->position < POS_FIGHTING )
	ch_roll = ch_roll * 3/4;

    /* special stance */
    if ( ch->stance == STANCE_DIMENSIONAL_BLADE && is_normal_hit(dt) )
	 ch_roll *= 2;

    if ( ch_roll <= 0 )
	return FALSE;
    else if ( victim_roll <= 0 )
	return TRUE;
    else
	return number_range(0, ch_roll) > number_range(0, victim_roll);
}

void aura_damage( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield, int dam )
{
    int level;

    if ( !IS_AFFECTED(victim, AFF_ELEMENTAL_SHIELD)
	 || (wield != NULL && (wield->value[0]==WEAPON_GUN
			       || wield->value[0]==WEAPON_BOW )) )
	return;

    if ( is_affected(victim, gsn_immolation) )
    {
/* This check added to each aura for reduced damage from Fervent Rage -Astark */
        if ( is_affected(ch, gsn_fervent_rage) )
        {
            level = affect_level( victim, gsn_immolation );
   	    full_dam(victim, ch, level/2, gsn_immolation, DAM_FIRE, TRUE);
        }
        else
        {
        level = affect_level( victim, gsn_immolation );
	full_dam(victim, ch, level, gsn_immolation, DAM_FIRE, TRUE);
        }
    }
    else if ( is_affected(victim, gsn_epidemic) )
    {
        if ( is_affected(ch, gsn_fervent_rage) )
        {
            level = affect_level( victim, gsn_epidemic );
   	    full_dam(victim, ch, level/2, gsn_epidemic, DAM_DISEASE, TRUE);
        }
        else
        {
	level = affect_level( victim, gsn_epidemic );
	full_dam(victim, ch, level, gsn_epidemic, DAM_DISEASE, TRUE);
        }
    }
    else if ( is_affected(victim, gsn_electrocution) )
    {
        if ( is_affected(ch, gsn_fervent_rage) )
        {
            level = affect_level( victim, gsn_electrocution );
   	    full_dam(victim, ch, level/2, gsn_electrocution, DAM_LIGHTNING, TRUE);
        }
        else
        {
	level = affect_level( victim, gsn_electrocution );
	full_dam(victim, ch, level, gsn_electrocution, DAM_LIGHTNING, TRUE);
        }
    } 
    else if ( is_affected(victim, gsn_absolute_zero) )
    {
        if ( is_affected(ch, gsn_fervent_rage) )
        {
            level = affect_level( victim, gsn_absolute_zero );
   	    full_dam(victim, ch, level/2, gsn_absolute_zero, DAM_COLD, TRUE);
        }
        else
        {
	level = affect_level( victim, gsn_absolute_zero );
	full_dam(victim, ch, level, gsn_absolute_zero, DAM_COLD, TRUE);
        }
    }
    /*added in by Korinn 1-19-99*/
    else if ( is_affected(victim, gsn_quirkys_insanity) )
    {
	level = affect_level( victim, gsn_quirkys_insanity );
	if ( number_bits(2) == 0 )
	{	  
	    int confusion_num, laughing_fit_num;
	    confusion_num=skill_lookup("confusion");
	    laughing_fit_num=skill_lookup("laughing fit");
	    
	    if (number_percent()<=10)
		spell_confusion(confusion_num, level/2, ch,(void *) ch, TARGET_CHAR);
	    else
		spell_laughing_fit(laughing_fit_num, level/2, ch,(void *) ch, TARGET_CHAR);
	}
	full_dam(victim, ch, level/2, gsn_quirkys_insanity, DAM_MENTAL, TRUE);
    }
} /* aura damage */

void stance_after_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield )
{
    int dam, old_wait, dt;

    if ( ch->stance == 0 )
	return;

    dam = 10 + number_range(ch->level, ch->level*2);

    switch ( ch->stance )
    {
    case STANCE_RHINO:
	if (number_bits(2) != 0) 
	    break;
	old_wait = ch->wait;
	if ( get_skill(ch, gsn_bash) > 0 )
	    do_bash(ch, "\0");
	ch->wait = old_wait;
	break;
    case STANCE_SCORPION:
	if (number_bits(2)==0)
	    poison_effect((void *)victim, ch->level, number_range(3,8), TARGET_CHAR);
	break;
    case STANCE_EEL:
	if (number_bits(2)==0)
	    shock_effect((void *)victim, ch->level, number_range(3,8), TARGET_CHAR);
	break;
    case STANCE_DRAGON:
	if (number_bits(2)==0)
	    fire_effect((void *)victim, ch->level, number_range(3,8), TARGET_CHAR);
	break;
    case STANCE_BOA:
	if (number_bits(5)==0) 
	    disarm(ch, victim, FALSE);
	if (number_bits(4)==0)
	{
	    send_to_char("You are too constricted to move.\n\r", victim);
	    act("$n is constricted and unable to move.", victim,NULL,NULL,TO_ROOM);
	    WAIT_STATE(victim, 2*PULSE_VIOLENCE);
	}
	break;
    case STANCE_SHADOWESSENCE:
	if (number_bits(2)==0)
	    cold_effect((void *)victim, ch->level, number_range(3,8), TARGET_CHAR);
	break;
    case STANCE_SHADOWCLAW:
	if (wield == NULL && number_bits(10)==69)
	{
	    act("In a mighty strike, $N's hand separates $n's neck.",
		victim,NULL,ch,TO_ROOM);
	    act("$N slashes $S hand through your neck.",victim,NULL,ch,TO_CHAR);
	    behead(ch, victim);
	}
	break;
    case STANCE_SHADOWSOUL:
	dam = dice(4, 4);
	act_gag("You draw life from $n.",victim,NULL,ch,TO_VICT, GAG_WFLAG);
	act_gag("You feel $N drawing your life away.",
		victim,NULL,ch,TO_CHAR, GAG_WFLAG);
	damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
	if (number_bits(6) == 0)
	    drop_align( ch );
	ch->hit += dam;
	break;
    case STANCE_VAMPIRE_HUNTING:
	if ((IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
	    || IS_SET(victim->vuln_flags, VULN_WOOD)
	    || IS_SET(victim->form, FORM_UNDEAD))
	    full_dam(ch, victim, dam, gsn_vampire_hunting, DAM_HOLY, TRUE);
	break;
    case STANCE_WEREWOLF_HUNTING:
	if (check_immune(victim,DAM_LIGHT)==IS_VULNERABLE
	    || IS_SET(victim->vuln_flags, VULN_SILVER))
	    full_dam(ch, victim, dam, gsn_werewolf_hunting, DAM_LIGHT, TRUE);
	break;
    case STANCE_WITCH_HUNTING:
	if ( (IS_NPC(victim) && IS_SET(victim->act,ACT_MAGE))
	     || (!IS_NPC(victim) && 
		 (victim->class==3||victim->class==11||victim->class==14)))
	    full_dam(ch, victim, dam, gsn_witch_hunting, DAM_DROWNING, TRUE);
	break;
    case STANCE_ELEMENTAL_BLADE:
	/* additional mana cost */
	if ( ch->mana < 1 )
	    break;
	else
	    ch->mana -= 1;
	/* if weapon damage can be matched.. */
	if ( wield != NULL )
	{
	    dt = attack_table[wield->value[3]].damage;
	    if ( dt == DAM_FIRE
		 || dt == DAM_COLD
		 || dt == DAM_ACID
		 || dt == DAM_LIGHTNING )
	    {
		full_dam(ch, victim, dam, gsn_elemental_blade, dt, TRUE);
		break;
	    }
	}
	/* ..else random damage type */
	switch( number_bits(2) )
	{
	case 0: dt = DAM_FIRE; break;
	case 1: dt = DAM_COLD; break;
	case 2: dt = DAM_ACID; break;
	case 3: dt = DAM_LIGHTNING; break;
	}
	full_dam(ch, victim, dam, gsn_elemental_blade, dt, TRUE);
	break;
    default:
	break;
    }
} /* stance_after_hit */

void weapon_flag_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield )
{
    int dam, flag;

    if ( wield == NULL )
	return;
	

    if ( IS_WEAPON_STAT(wield, WEAPON_POISON) )
    {
	if ( number_bits(1) == 0 )
	    poison_effect( (void *) victim, wield->level/2, 0, TARGET_CHAR);
	CHECK_RETURN( ch, victim );
    } 

    if ( IS_WEAPON_STAT(wield, WEAPON_PARALYSIS_POISON) )
    {
	if ( number_bits(1) == 0 )
	    paralysis_effect( (void *) victim, wield->level/3*2, 0, TARGET_CHAR);
	CHECK_RETURN( ch, victim );
    }
	
    if ( IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
    {
	dam = dice( 2, 4 );
	act_gag("$p draws life from $n.",victim,wield,NULL,TO_ROOM, GAG_WFLAG);
	act_gag("You feel $p drawing your life away.",
		victim,wield,NULL,TO_CHAR, GAG_WFLAG);
	damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
	CHECK_RETURN( ch, victim );
	if (number_bits(6) == 0)
	    drop_align( ch );
	ch->hit += dam/2;
    }
	
    if ( IS_WEAPON_STAT(wield,WEAPON_MANASUCK))
    {
	dam = dice( 2, 4 );
	act_gag("$p envelops $n in a blue mist, drawing $s magical energy away!",
		victim,wield,NULL,TO_ROOM, GAG_WFLAG);
	act_gag("$p envelops you in a blue mist, drawing away your magical energy!",
		victim,wield,NULL,TO_CHAR, GAG_WFLAG);
	damage(ch,victim,dam,0,DAM_DROWNING,FALSE);
	CHECK_RETURN( ch, victim );
	if (number_bits(6) == 0)
	    drop_align( ch );
	ch->mana += dam/2;
    }
	
    if ( IS_WEAPON_STAT(wield,WEAPON_MOVESUCK))
    {
	dam = dice( 2, 4 );
	act_gag("$p drives deep into $n, sucking away $s will to fight!",
		victim,wield,NULL,TO_ROOM, GAG_WFLAG);
	act_gag("$p drives deep inside you, sucking away your will to fight!",
		victim,wield,NULL,TO_CHAR, GAG_WFLAG);
	damage(ch,victim,dam,0,DAM_COLD,FALSE);
	CHECK_RETURN( ch, victim );
	if (number_bits(6) == 0)
	    drop_align( ch );
	ch->move += dam/2;
    }
	
    if ( IS_WEAPON_STAT(wield,WEAPON_DUMB))
    {
	dam = dice( 1, 4 );
	act_gag("$p breaks $n's train of thought!",victim,wield,NULL,TO_ROOM, GAG_WFLAG);
	act_gag("UuuhNnNNhhh. You feel $p sucking away your brain power.",
		victim,wield,NULL,TO_CHAR, GAG_WFLAG);
	if ( !number_bits(2) )
	    dumb_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
	damage(ch,victim,dam,0,DAM_SOUND,FALSE);
	CHECK_RETURN( ch, victim );
    }
	
    if ( IS_WEAPON_STAT(wield,WEAPON_FLAMING))
    {
	dam = dice( 1, 4 );
	act_gag("$n is burned by $p.",victim,wield,NULL,TO_ROOM, GAG_WFLAG);
	act_gag("$p sears your flesh.",victim,wield,NULL,TO_CHAR, GAG_WFLAG);
	if ( !number_bits(2) )
	    fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
	damage(ch,victim,dam,0,DAM_FIRE,FALSE);
	CHECK_RETURN( ch, victim );
    }
	
    if ( IS_WEAPON_STAT(wield,WEAPON_FROST))
    {
	dam = dice( 1, 4 );
	act_gag("$p freezes $n.",victim,wield,NULL,TO_ROOM, GAG_WFLAG);
	act_gag("The cold touch of $p surrounds you with ice.",
		victim,wield,NULL,TO_CHAR, GAG_WFLAG);
	if ( !number_bits(2) )
	    cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
	damage(ch,victim,dam,0,DAM_COLD,FALSE);
	CHECK_RETURN( ch, victim );
    }
	
    if ( IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
    {
	dam = dice( 1, 4 );
	act_gag("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM, GAG_WFLAG);
	act_gag("You are shocked by $p.",victim,wield,NULL,TO_CHAR, GAG_WFLAG);
	if ( !number_bits(2) )
	    shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
	damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
	CHECK_RETURN( ch, victim );
    }

    if ( IS_WEAPON_STAT(wield, WEAPON_PUNCTURE) )
    {
	AFFECT_DATA af;

	af.where    = TO_AFFECTS;
	af.type     = gsn_puncture;
	af.level    = wield->level;
	af.duration = 10;
	af.location = APPLY_AC;
	af.modifier = 1;
	af.bitvector = 0;
	affect_join(victim,&af);

	act_gag("$n's armor is damaged by $p.",
		victim,wield,NULL,TO_ROOM, GAG_WFLAG);
	act_gag("Your armor is damaged by $p.",
		victim,wield,NULL,TO_CHAR, GAG_WFLAG);
    }

  /* New weapon flag added by Astark 12-28-12. Chance to cast 3 different
     spells with each hit.. low chance. Heavy damage */
    if ( IS_WEAPON_STAT(wield,WEAPON_STORMING))
    {
        int level = wield->level;
        dam = number_range(10 + wield->level*3/4, 10 + wield->level*4/3);
        if (!number_bits(4))
   	    full_dam(ch,victim,dam,gsn_lightning_bolt, DAM_LIGHTNING, TRUE);

        if (!number_bits(4))
            full_dam(ch,victim,dam,gsn_meteor_swarm, DAM_FIRE, TRUE);

        if (!number_bits(4))
            full_dam(ch,victim,dam,gsn_monsoon, DAM_DROWNING, TRUE);

        CHECK_RETURN( ch, victim);
    }




    /* remove temporary weapon flags 
     * also solves problem with old weapons with different flags
     */
    for ( flag = 0; weapon_type2[flag].name != NULL; flag++ )
    {
	int bit = weapon_type2[flag].bit;

	if ( wield->pIndexData->vnum == OBJ_VNUM_BLACKSMITH
	     && (bit == WEAPON_SHARP || bit == WEAPON_VORPAL)
	     || bit == WEAPON_TWO_HANDS )
	    continue;

	if ( number_bits(10) == 0
	     && IS_WEAPON_STAT(wield, bit)
	     && !IS_WEAPON_STAT(wield->pIndexData, bit) )
	{
	    char buf[MSL];
	    sprintf( buf, "The %s effect on $p has worn off.", 
		     weapon_type2[flag].name );
	    act( buf, ch, wield, NULL, TO_CHAR );
	    I_REMOVE_BIT(wield->value[4], bit );
	}
    }

    /* maces can dazzle --Bobble */
    if ( (wield->value[0] == WEAPON_MACE
	  || wield->value[0] == WEAPON_FLAIL)
	 && number_bits(6) == 0 )
    {
	act( "You hit $N on the head, leaving $M dazzled.", ch, NULL, victim, TO_CHAR );
	act( "$n hits you on the head, leaving you dazzled.", ch, NULL, victim, TO_VICT );
	act( "$n hits $N on the head, leaving $M dazzled.", ch, NULL, victim, TO_NOTVICT );
	DAZE_STATE( victim, 2 * PULSE_VIOLENCE );
    }

} /* weapon_flag_hit */

void check_behead( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield )
{
    int chance = 0;

    if ( number_bits(10) != 69 )
	return;

    if ( wield == NULL )
    {
	if ( !IS_NPC(ch) && /* active AND passive skill => mobs have it */
	     ( number_percent() <= get_skill(ch, gsn_razor_claws)
	     || (number_range(0,1) && ch->stance == STANCE_SHADOWCLAW) ) )
	{
	    act("In a mighty strike, your claws separate $N's neck.",
		ch,NULL,victim,TO_CHAR);
	    act("In a mighty strike, $n's claws separate $N's neck.",
		ch,NULL,victim,TO_NOTVICT);
	    act("$n slashes $s claws through your neck.",ch,NULL,victim,TO_VICT);
	    behead(ch, victim);
	    check_improve(ch, gsn_razor_claws, 0, TRUE);
	}
	else
	    check_improve(ch, gsn_razor_claws, 0, FALSE);
	return;
    }

    switch ( wield->value[0] )
    {
    case WEAPON_EXOTIC:
	chance = 0;
	break;
    case WEAPON_DAGGER:
    case WEAPON_POLEARM:
	chance = 1;
	break;
    case WEAPON_SWORD:
	chance = 5;
	break;
    case WEAPON_AXE:
	chance = 25;
	break;
    default:
	return;
    }
   
    if ( get_skill(ch, gsn_beheading) == 100)
        chance += 2;

    chance += get_skill(ch, gsn_beheading) / 2;
    if ( IS_WEAPON_STAT(wield, WEAPON_SHARP) ) 
	chance += 1;
    if ( IS_WEAPON_STAT(wield, WEAPON_VORPAL) )
	chance += 5;

    if ( number_percent() <= chance
	 || ch->stance == STANCE_SHADOWCLAW )
    {
	act("$n's head is separated from his shoulders by $p.",
	    victim,wield,NULL,TO_ROOM);
	act("Your head is separated from your shoulders by $p.",
	    victim,wield,NULL,TO_CHAR);
	check_improve(ch, gsn_beheading, 0, TRUE);
	behead(ch, victim);
    }
    else
	check_improve(ch, gsn_beheading, 0, FALSE);
}

void check_assassinate( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield, int chance )
{
    int extra_chance;

    if ( wield == NULL || wield->value[0] != WEAPON_DAGGER )
	return;

    extra_chance = 50 + get_skill( ch, gsn_anatomy ) / 4;
    if ( IS_WEAPON_STAT(wield, WEAPON_VORPAL) )
	extra_chance += 10;

    if ( get_skill(ch, gsn_assassination) == 100)
        extra_chance += 2;


    if ( number_bits(chance) == 0
	 && number_percent() <= extra_chance
	 && (ch->stance == STANCE_AMBUSH || number_bits(1))
	 && number_percent() <= get_skill(ch, gsn_assassination) )
    {
	act("You sneak up behind $N, and slash $S throat!",ch,NULL,victim,TO_CHAR);
	act("$n sneaks up behind you and slashes your throat!",ch,NULL,victim,TO_VICT);
	act("$n sneaks up behind $N, and slashes $S throat!",ch,NULL,victim,TO_NOTVICT);
	behead(ch,victim);
	check_improve(ch,gsn_assassination,TRUE,0);
    }
    else
	check_improve(ch,gsn_assassination,FALSE,3);
}

/* adjust damage according to imm/res/vuln of ch 
 * dam_type must not be a mixed damage type
 * also adjusts for forst_aura etc.
 */
int adjust_damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dam_type)
{
    if ( IS_SET(ch->form, FORM_FROST) )
	if ( dam_type == DAM_COLD )
	    dam += dam/3;
	else if ( dam_type == DAM_FIRE )
	    dam -= dam/4;
    if ( IS_SET(ch->form, FORM_BURN) )
	if ( dam_type == DAM_FIRE )
	    dam += dam/3;
	else if ( dam_type == DAM_COLD )
	    dam -= dam/4;

    switch(check_immune(victim,dam_type))
    {
    case(IS_IMMUNE):
        return 0;
    case(IS_RESISTANT): 
        return dam - dam/4;
    case(IS_VULNERABLE):
        return dam + dam/3;
    default: 
        return dam;
    }
}

/* returns wether dt = damage type indicates a 'normal' attack that
 * can be dodged etc.
 */
bool is_normal_hit( int dt )
{
    return (dt >= TYPE_HIT)
     || (dt == gsn_double_strike)
     || (dt == gsn_strafe)
     || (dt == gsn_round_swing)
     || (dt == gsn_burst)
     || (dt == gsn_fullauto)
     || (dt == gsn_semiauto)
     || (dt == gsn_pistol_whip);
}

/* deal direct, unmodified, non-lethal damage */
void direct_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int sn )
{
    dam = URANGE( 0, dam, victim->hit - 1 );

    victim->hit -= dam;
    remember_attack(victim, ch, dam);
    if ( sn > 0 && dam > 0 )
	dam_message(ch,victim,dam,sn,FALSE);
}

/*
* Inflict reduced damage from a hit.
*/
bool damage( CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,
	     bool show )
{
    /* damage reduction */
    if ( dam > 40)
    {
        dam = (2*dam - 80)/3 + 40;
        if ( dam > 80)
        {
            dam = (2*dam - 160)/3 + 80; 
            if ( dam > 160)
            {
                dam = (2*dam-320)/3 + 160;
                if (dam>320)
                    dam=(2*dam-640)/3 + 320;
            }           
        }
    }

    return full_dam( ch, victim, dam, dt, dam_type, show );
}

/*
* Inflict full damage from a hit.
*/
bool full_dam( CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,
	     bool show ) 
{
    bool immune;
    char buf[MAX_STRING_LENGTH];
    int stance = ch->stance;
    int diff;
    
    if ( stop_attack(ch, victim) )
        return FALSE;
    
    /*
    * Stop up any residual loopholes.
    */
    if ( dam > 2000 && dt >= TYPE_HIT && !IS_IMMORTAL(ch))
    {
        OBJ_DATA *obj;
        bug( "Damage: %d: more than 2000 points!", dam );
        dam = 2000;
        obj = get_eq_char( ch, WEAR_WIELD );
        send_to_char("You really shouldn't cheat.\n\r",ch);
        if (obj != NULL)
            extract_obj(obj);
    }
    
    if ( victim != ch )
    {
    /*
    * Certain attacks are forbidden.
    * Most other attacks are returned.
        */
        if ( is_safe( ch, victim ) )
            return FALSE;
        
        check_killer( ch, victim );
        
        if ( victim->position > POS_STUNNED )
        {
            if ( victim->fighting == NULL )
            {
                set_fighting( victim, ch );
		/* trick to make mob peace work properly */
		if ( victim->fighting == NULL )
		    return FALSE;
		/*
		if ( check_kill_trigger(ch, victim) )
		    return FALSE;
		*/
            }
	    /* what has timer to do with getting up?
            if (victim->timer <= 4)
		set_pos( victim, POS_FIGHTING );
	    */
        }
        
        if ( victim->position > POS_STUNNED )
        {
            if ( ch->fighting == NULL )
            {
                set_fighting( ch, victim );
            }
            
            if ( IS_NPC(ch)
                &&   IS_NPC(victim)
                &&   IS_AFFECTED(victim, AFF_CHARM)
                &&   victim->master != NULL
                &&   victim->master->in_room == ch->in_room
                &&   number_bits( 6 ) == 0 )
            {
                stop_fighting( ch, FALSE );
                multi_hit( ch, victim->master, TYPE_UNDEFINED );
                return FALSE;
            }
        }
        
        /*
        * More charm stuff.
        */
        if ( victim->master == ch )
            stop_follower( victim );
    
	/*
	 * Inviso attacks ... not.
	 */
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
	{
	    affect_strip( ch, gsn_invis );
	    affect_strip( ch, gsn_mass_invis );
	    REMOVE_BIT( ch->affect_field, AFF_INVISIBLE );
	    act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
	}
    
	/*
	 * Astral attacks ... not.
	 */
	if ( IS_AFFECTED(ch, AFF_ASTRAL) )
	{
	    affect_strip( ch, gsn_astral );
	    REMOVE_BIT( ch->affect_field, AFF_ASTRAL );
	    act( "$n returns to the material plane.", ch, NULL, NULL, TO_ROOM );
	}
	
	/*
	 * Sheltered attacks ... not.
	 */
	if ( IS_AFFECTED(ch, AFF_SHELTER) )
	{
	    affect_strip( ch, gsn_shelter );
	    REMOVE_BIT( ch->affect_field, AFF_SHELTER );
	    act( "$n is no longer sheltered!", ch, NULL, NULL, TO_ROOM );
	}
    
	/*
	 * Hidden attacks ... not.
	 */ 
	if ( IS_AFFECTED(ch, AFF_HIDE) )
	{
	    affect_strip( ch, gsn_hide );
	    REMOVE_BIT( ch->affect_field, AFF_HIDE );
	    act( "$n leaps out of hiding!",ch,NULL,NULL,TO_ROOM );
	}

	/* no mimic attacks */
	/*
	if ( is_affected(ch, gsn_mimic) )
	{
	    affect_strip( ch, gsn_mimic );
	    act( "The illusion surrounding $n fades.", ch, NULL, NULL, TO_ROOM );
	}
	*/
    
    } /* if ( ch != victim ) */

    /* another safety-net */
    if ( stop_attack(ch, victim) )
        return FALSE;

   /*
    * Damage modifiers.
    */
    
    if ( dam > 1 && !IS_NPC(victim) && victim->pcdata->condition[COND_DRUNK] > 10 )
        dam = 9 * dam / 10;
    
    if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
/* Removed 3/8/01.  -Rimbol
       if (victim->stance == STANCE_BLADE_DANCE)
            dam = (dam * 3) / 4;
        else
*/
	if ( stance == STANCE_UNICORN && dt >= TYPE_HIT )
	    dam = dam * 5/6;
	else
            dam /= 2;
    }
    
    if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
        ||           (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
    {
	diff = UMAX(ch->alignment,victim->alignment) - UMIN(ch->alignment,victim->alignment);
	if( diff > 999 )
	    dam -= dam / 4;
	else
	    dam -= diff*dam/4000;
    }

    if ( victim->stance == STANCE_ARCANA && dt < TYPE_HIT && IS_SPELL(dt) )
	dam -= dam/3;
    
    immune = FALSE;
    
    /*
    * Check for parry, dodge, etc. and fade
    */
    if ( is_normal_hit(dt) && check_avoid_hit(ch, victim, show) )
	return FALSE;

    /* check imm/res/vuln for single & mixed dam types */
    if ( dam > 0 )
	if (IS_MIXED_DAMAGE(dam_type))
	{
	    int first_dam_type = FIRST_DAMAGE(dam_type);
	    int second_dam_type = SECOND_DAMAGE(dam_type);
	    dam = adjust_damage(ch, victim, dam / 2, first_dam_type) +
		adjust_damage(ch, victim, (dam+1) / 2, second_dam_type);
	    immune = ((check_immune(victim, first_dam_type) == IS_IMMUNE)
		      && (check_immune(victim, second_dam_type) == IS_IMMUNE));
	}
	else
	{
	    dam = adjust_damage(ch, victim, dam, dam_type);
	    immune = (check_immune(victim, dam_type) == IS_IMMUNE);
	}

    if (dt == gsn_beheading)
    {
        immune = FALSE;
        dam = victim->hit + 100;
    }
    
    if ( dam > 0 && is_normal_hit(dt) )
    {
	if ( stance != 0 )
        if (   stance == STANCE_BEAR 
            || stance == STANCE_DRAGON
            || stance == STANCE_LION
            || stance == STANCE_RHINO
            || stance == STANCE_RAGE
            || stance == STANCE_BLADE_DANCE
            || stance == STANCE_DIMENSIONAL_BLADE
            || stance == STANCE_KORINNS_INSPIRATION 
            || stance == STANCE_PARADEMIAS_BILE )
            dam += 5 + dam / 5;
        else if ( stance == STANCE_SERPENT )
            dam += dam / 4;
        else if ( stance == STANCE_AVERSION )
            dam = dam * 3/4;
        else if ( stance == STANCE_BLOODBATH )
            dam += (20 + dam) / 2;
	else if ( stance == STANCE_KAMIKAZE )
	    dam += (18 + dam) / 3;
        else if ( stance == STANCE_GOBLINCLEAVER )
            dam = dam * 2/3;
	else if ( stance == STANCE_WENDIGO )
	    dam += 2 + dam/10;
	else if ( stance == STANCE_EEL )
	    dam += 3 + dam/6;
	else if ( stance == STANCE_SCORPION
		  || stance == STANCE_SHADOWESSENCE )
	    dam += 2 + dam/10;

	/* victim stance can influence damage too */
	if ( victim->stance == STANCE_BLOODBATH )
	    dam += (20 + dam) / 4; 
	else if ( victim->stance == STANCE_KAMIKAZE )
	    dam += (18 + dam) / 6;
	else if ( victim->stance == STANCE_TORTOISE ) 
	    dam -= dam / 3;
	else if ( victim->stance == STANCE_WENDIGO )
	    dam -= dam / 5;
    }

    /* religion bonus */
    /*
    if ( ch != victim )
	dam += dam * get_religion_bonus(ch) / 100;
    */

    if (show)
        dam_message( ch, victim, dam, dt, immune );
    
    if (dam == 0)
        return FALSE;
    
    if ( is_affected(victim, gsn_disguise)
	 && chance( 100 * dam / victim->level )
	 && number_bits(2) == 0 )
    {
	affect_strip( victim, gsn_disguise );
	act( "Your disguise is torn apart!", victim, NULL, NULL, TO_CHAR );
	act( "The disguise of $n is torn apart!", victim, NULL, NULL, TO_ROOM );
    }

    /* iron maiden returns part of the damage */
    if ( IS_AFFECTED(ch, AFF_IRON_MAIDEN) && ch != victim )
    {
	int iron_dam;
        int dam_cap;

 /* Added the IS_NPC(ch) check because it wasn't working properly - Astark 1-6-13 */
        if (IS_NPC(victim) && IS_NPC(ch))
            dam = 0;

	if ( IS_NPC(ch) )
	    iron_dam = dam/2;
	else
	    iron_dam = dam/4;

        if (IS_NPC(ch))
            iron_dam = URANGE(0, iron_dam, 100);

	/* if-check to lower spam */
	if ( show || iron_dam > ch->level )
	    direct_damage( ch, ch, iron_dam, skill_lookup("iron maiden") );
	else
	    direct_damage( ch, ch, iron_dam, 0 );
    }



    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    
    if ( dt != gsn_beheading && !IS_AFFECTED(victim, AFF_MANA_BURN) )
    {
	if ( victim->stance == STANCE_PHOENIX )
	{
	    if (victim->mana < dam / 2)
	    {
		dam -= victim->mana * 2;
		victim->mana = 0;
	    }
	    else
	    {
		victim->mana -= dam/2;

		/* Tweak to make phoenix less uberpowerful:
		 *	it works as it once did, if you are below 25% of your max hp,
		 *	but it works like mana shield, if you are at full hp,
		 *	with a gradual gradient between the two extremes.
		 */
		if( victim->hit < victim->max_hit/4 ) dam = 0;
		else dam -= (victim->hit/victim->max_hit - .25) * 2/3;
	    }
	}

	if ( is_affected(victim, gsn_mana_shield) )
	{
	    int mana_loss = UMIN(dam / 2, victim->mana);
	    victim->mana -= mana_loss;
	    dam -= mana_loss;
	}
    }

    victim->hit -= dam;
    remember_attack(victim, ch, dam);
    
    /* deaths door check Added by Tryste */
    if ( !IS_NPC(victim) && victim->hit < 1 )
    {
        if (IS_AFFECTED(victim, AFF_DEATHS_DOOR) )
        {
            if ( number_percent() <= 25 )
            {
                stop_fighting(victim, TRUE);
                victim->hit = 100 + victim->level * 10;
                victim->move  += 100;
                send_to_char("The gods have protected you from dying!\n\r", victim);
		act( "The gods resurrect $n.", victim, NULL, NULL, TO_ROOM );
            }
            else
                send_to_char("Looks like you're outta luck!\n\r", victim);
            /*REMOVE_BIT(victim->affect_field, AFF_DEATHS_DOOR);*/
            affect_strip(victim, gsn_deaths_door);
        }
    }   
    
    if ( !IS_NPC(victim)
        &&   victim->level >= LEVEL_IMMORTAL
        &&   victim->hit < 1 )
        victim->hit = 1;
    
    if (!IS_NPC(victim) && NOT_AUTHED(victim) && victim->hit < 1)
        victim->hit = 1;
    
    /* no breaking out of jail by dying.. */
    if ( !IS_NPC(victim) 
	 && IS_SET(victim->penalty, PENALTY_JAIL)
	 && victim->hit < 1 )
    {
	send_to_char( "Your sentence is jail, not death!\n\r", victim );
	victim->hit = 1;
    }

    update_pos( victim );
    
    switch( victim->position )
    {
    case POS_MORTAL:
        act( "$n is mortally wounded, and will die soon, if not aided.",
            victim, NULL, NULL, TO_ROOM );
        send_to_char( 
            "You are mortally wounded, and will die soon, if not aided.\n\r",
            victim );
        break;
        
    case POS_INCAP:
        act( "$n is incapacitated and will slowly die, if not aided.",
            victim, NULL, NULL, TO_ROOM );
        send_to_char(
            "You are incapacitated and will slowly die, if not aided.\n\r",
            victim );
        break;
        
    case POS_STUNNED:
        act( "$n is stunned, but will probably recover.",
            victim, NULL, NULL, TO_ROOM );
        send_to_char("You are stunned, but will probably recover.\n\r",
            victim );
        break;
        
    case POS_DEAD:
        if (victim!=ch || !(!IS_NPC(victim) && IS_SET(victim->act, PLR_WAR)))
        {
            
            act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
            send_to_char( "You have been KILLED!!\n\r\n\r", victim );
            
            if (!IS_NPC(victim) && !IS_SET( victim->act, PLR_WAR)) 
	    {
	        if( IS_NPC(ch) )
                    victim->pcdata->mob_deaths++;
		else
                    victim->pcdata->pkill_deaths++;
	    }
        }
        else
        {
            send_to_char( "Coming so close to a senseless death has re-energized you and you leap back to your feet!\n\r", victim );
            ch->hit = 10;
	    set_pos( ch, POS_STANDING );
        }
        break;
        
    default:
        if ( dam > victim->max_hit / 4 )
            send_to_char( "That really did HURT!\n\r", victim );
        if ( victim->hit < victim->max_hit / 4 
            && !IS_SET(victim->gag, GAG_BLEED))
            send_to_char( "You sure are BLEEDING!\n\r", victim );
        break;
    }
    
    /*
    * Sleep spells and extremely wounded folks.
    */
    if ( !IS_AWAKE(victim) && (victim != ch || victim->position <= POS_STUNNED))
	stop_fighting( victim, TRUE );
    
    if ( !IS_NPC(victim) && victim->hit < 1)
    {
        CHAR_DATA *m;
        
        for (m = ch->in_room->people; m != NULL; m = m->next_in_room)
            if (IS_NPC(m) && HAS_TRIGGER(m, TRIG_DEFEAT))
            {
                mp_percent_trigger( m, victim, NULL, NULL, TRIG_DEFEAT );
                victim->hit = 1;
		set_pos( victim, POS_STUNNED );
                return FALSE;
            }
    }
    
    /*
    * Payoff for killing things.
    */
    if ( victim->position == POS_DEAD )
    {
	handle_death( ch, victim );
        return TRUE;
    }
   
   if ( victim == ch )
       return TRUE;
   
   /*
    * Take care of link dead people.
    */
   if ( !IS_NPC(victim) && !IS_IMMORTAL(victim) && victim->desc == NULL )
   {
       if ( victim->position >= POS_FIGHTING 
	    && victim->wait == 0 )
       {
           do_recall( victim, "" );
           return TRUE;
       }
   }
   
   /*
   * Wimp out?
   */
   if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
   {
       if ( ((IS_SET(victim->act, ACT_WIMPY) || IS_AFFECTED(victim, AFF_FEAR))
	     && !IS_SET(victim->act, ACT_SENTINEL)
	     && number_bits( 2 ) == 0
	     && (victim->hit < victim->max_hit / 5 
		 || (IS_AFFECTED(victim, AFF_INSANE) && number_bits(6) == 0)
		 || IS_AFFECTED(victim, AFF_FEAR)))
	    || ( IS_AFFECTED(victim, AFF_CHARM) 
		 && victim->master != NULL
		 && victim->master->in_room != victim->in_room ) )
           do_flee( victim, "" );
   }
   
   if ( !IS_NPC(victim)
	&& victim->hit > 0
	&& !IS_SET(victim->act, PLR_WAR)
	&& ( victim->hit <= victim->wimpy
	     || (IS_AFFECTED(victim, AFF_INSANE) && number_bits(6)==0)
	     || IS_AFFECTED(victim, AFF_FEAR) ) 
	&& victim->wait < PULSE_VIOLENCE / 2 )
       do_flee( victim, "" );
   
   tail_chain( );
   return TRUE;
}

/* previously part of method damage --Bobble */
void handle_death( CHAR_DATA *ch, CHAR_DATA *victim )
{
    static bool recursion_check = FALSE;
    char buf[MSL];
    OBJ_DATA *corpse;
    bool killed_in_war = FALSE;
    bool morgue = FALSE;

    /* safety-net */
    if ( recursion_check )
    {
	bugf( "handle_death: recursive kill" );
        return;
    }
    recursion_check = TRUE;

    /* Clan counters */
    if (IS_NPC(ch) && IS_NPC(victim) || ch == victim)
	; /* No counter */
    else if (IS_NPC(ch) && !IS_NPC(victim))
    {
	clan_table[victim->clan].mobdeaths++;
	clan_table[victim->clan].changed = TRUE;
    }
    else if (!IS_NPC(ch) && IS_NPC(victim))
    {
	clan_table[ch->clan].mobkills++;
	clan_table[ch->clan].changed = TRUE;
    }
    else if (!IS_NPC(ch) && !IS_NPC(victim))
    {
	clan_table[ch->clan].pkills++;
	clan_table[ch->clan].changed = TRUE;
	clan_table[victim->clan].pdeaths++;
	clan_table[victim->clan].changed = TRUE;
    }

    if ( IS_NPC(ch) )
	forget_attacker(ch, victim);
        
    if ( !PLR_ACT(ch, PLR_WAR) )
	group_gain( ch, victim );

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	ch->pcdata->mob_kills++;
        check_achievement(ch);
    }

    /*
    if (!IS_NPC(ch) && !IS_SET(ch->act, PLR_WAR))
    {
	group_gain( ch, victim );
	if (IS_NPC(victim))   
	    ch->pcdata->mob_kills++;
    }
    */
    
    check_kill_quest_completed( ch, victim );
    stop_singing(victim);
        
    if (!IS_NPC(victim))
    {
	sprintf( log_buf, "%s killed by %s at %d",
		 victim->name,
		 (IS_NPC(ch) ? ch->short_descr : ch->name),
		 ch->in_room->vnum );
	log_string( log_buf );
	
	if ( IS_SET( victim->act, PLR_WAR ) && IS_SET( ch->act, PLR_WAR ) )
        {
	    sprintf( buf, "%s has been slain by %s!\n\r", victim->name, ch->name );
	    warfare( buf );
	    
	    if ( victim != ch )
	    {
		add_war_kills( ch );
		adjust_wargrade( ch, victim );
	    }
	    
	    killed_in_war = TRUE;
	    
	    do_restore(victim, victim->name);
	}
            
	if (IS_NPC(ch))
        {                
	    sprintf(log_buf, "%s has been killed by %s!", victim->name, ch->short_descr);
	    info_message(victim, log_buf, TRUE);            
	}
	else if (ch != victim)
        {
	    if (!IS_SET(victim->act, PLR_WAR)) 
            {
		ch->pcdata->pkill_count++;
		adjust_pkgrade( ch, victim, FALSE );
		
		if (!clan_table[ch->clan].active)
                {
		    sprintf(log_buf, "%s has been pkilled by %s!",victim->name, ch->name);
		    info_message(victim, log_buf, TRUE);         
		}
		else
                {
		    CLANWAR_DATA *p;
		    
		    if ( clan_table[ch->clan].rank_list[ch->pcdata->clan_rank].clanwar_pkill == TRUE
			 && clan_table[victim->clan].rank_list[victim->pcdata->clan_rank].clanwar_pkill == TRUE
			 && (p = clanwar_lookup(ch->clan, victim->clan)) 
			 && p->status == CLANWAR_WAR)
                    {
			p->pkills++;
                            
			sprintf(log_buf, "%s has been pkilled by %s of clan %s!",
				victim->name, ch->name, capitalize(clan_table[ch->clan].name));
			info_message(victim, log_buf, TRUE);

			sprintf(log_buf, "Clan %s has now killed %d %s during the current war!",
                                capitalize(clan_table[ch->clan].name),
                                p->pkills,
                                capitalize(clan_table[victim->clan].name));
			info_message(NULL, log_buf, TRUE);
			save_clanwars();
		    }
		    else   // kill did not contribute to clanwar pkills, so don't mention clan (good for religion!)
		    {
			sprintf(log_buf, "%s has been pkilled by %s!",victim->name, ch->name);
			info_message(victim, log_buf, TRUE);
		    }
		}
	    }/*end is_set(plr) war check*/
	}
	else 
        {
	    if ( !IS_SET( victim->act, PLR_WAR ) )
            {
		sprintf(log_buf, "%s has carelessly gotten killed.", victim->name);
		info_message(NULL, log_buf, TRUE);
	    }
	}
    }
    
        
    sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
	     (IS_NPC(victim) ? victim->short_descr : victim->name),
	     (IS_NPC(ch) ? ch->short_descr : ch->name),
	     ch->in_room->name, ch->in_room->vnum);
    
    if (IS_NPC(victim))
	wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
    else
	wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
    
    if (!IS_NPC(victim)
	&& victim->pcdata->bounty > 0
	&& ch != victim
	&& !IS_SET( victim->act, PLR_WAR ))
    {
	if (!IS_NPC(ch))
        {
	    sprintf(buf,"You receive a %d gold bounty, for killing %s.\n\r",
                    victim->pcdata->bounty, victim->name);
	    send_to_char(buf, ch);
	    ch->gold += victim->pcdata->bounty;
	    victim->pcdata->bounty = 0;
	    update_bounty(victim);
	}
	/* If a non-pkill player is killed by an NPC bounty hunter */
	else if (IS_NPC(ch) && (ch->spec_fun == spec_lookup( "spec_bounty_hunter" ))
		 && (!IS_SET(victim->act, PLR_PERM_PKILL)) )
        {
	    int amount;
	    amount = UMAX(victim->pcdata->bounty / 5 + 10, victim->pcdata->bounty);
	    victim->pcdata->bounty -= amount;
	    ch->gold += amount;
	    amount = victim->gold / 10;
	    ch->gold += amount;
	    victim->gold -= amount;
	    amount = victim->silver / 10;
	    ch->silver += amount;
	    victim->silver -= amount;
	    update_bounty(victim);      
	}
    }
        
    /*
     * Death trigger
     */
    if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
    {
	set_pos( victim, POS_STANDING );
	mp_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH );
    }

    remort_remove(victim, FALSE);
        
    if ( IS_NPC(victim) || !IS_SET( victim->act, PLR_WAR ) )
    {
	morgue = (bool) (!IS_NPC(victim) && (IS_NPC(ch) || (ch==victim) ));


	raw_kill( victim, ch, morgue );
	
	/* dump the flags */
	if (ch != victim && !is_same_clan(ch,victim))
	{
	    REMOVE_BIT(victim->act,PLR_KILLER);
	    REMOVE_BIT(victim->act,PLR_THIEF);
	}
    }
    
    if ( killed_in_war )
	war_remove( victim, TRUE );
        
    /* RT new auto commands */
    
    if (!IS_NPC(ch)
	&& (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
	&&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse)
	&& !IS_SET( ch->act, PLR_WAR ))
    {
	OBJ_DATA *coins;
	
	corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 
	
	if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
	     corpse && corpse->contains) /* exists and not empty */
	    do_get( ch, "all room.corpse" );
	
	if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	    corpse && corpse->contains  && /* exists and not empty */
	    !IS_SET(ch->act,PLR_AUTOLOOT))
	    if ((coins = get_obj_list(ch,"gcash",corpse->contains)) != NULL)
		do_get(ch, "all.gcash room.corpse");
	
	if ( IS_SET(ch->act, PLR_AUTOSAC) )
	    if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		;  /* leave if corpse has treasure */
	    else
		do_sacrifice( ch, "corpse" );
    }

    if ( victim->pcdata != NULL && victim->pcdata->remorts==0 && morgue )
    {
	send_to_char( "HINT: You can retrieve lost items from your corpse.\n\r", victim );
	send_to_char( "      Check 'help corpse' for details.\n\r", victim );
    }

    victim->just_killed = TRUE;
    recursion_check = FALSE;
} 

bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
    return is_safe_check( ch, victim, FALSE, FALSE, FALSE );
}

bool is_safe_spell( CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    return is_safe_check( ch, victim, area, TRUE, FALSE );
}

bool is_always_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
    return is_safe_check( ch, victim, FALSE, TRUE, TRUE );
}

/* mama function for is_safe and is_safe_spell --Bobble */
bool is_safe_check( CHAR_DATA *ch, CHAR_DATA *victim, 
		    bool area, bool quiet, bool theory )
{
    bool ignore_safe =
	(IS_NPC(ch) && IS_SET(ch->act, ACT_IGNORE_SAFE))
	|| (IS_NPC(victim) && IS_SET(victim->act, ACT_IGNORE_SAFE));

    if ( !theory && (victim->in_room == NULL || ch->in_room == NULL) )
        return TRUE;
    
    if ( !theory && (victim->fighting == ch || ch->fighting == victim) )
        return FALSE;

    if ( ((victim == ch) || (is_same_group(victim,ch))) && area)
        return TRUE;
    
    if ( victim == ch )
	return FALSE;
    
    if ( IS_NPC(victim) && IS_SET(victim->act, ACT_OBJ) && area )
	return TRUE;

    if ( IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL && !area )
        return FALSE;

    if ( IS_IMMORTAL(victim) && !IS_IMMORTAL(ch) )
    {
	if ( !quiet )
	    send_to_char("You would get squashed!\n\r",ch);
        return TRUE;
    }

    /* fear */
    if ( !theory && IS_AFFECTED(ch, AFF_FEAR) )
    {
	if ( !quiet )
	    send_to_char("You're too scared to attack anyone!\n\r",ch);
        return TRUE;
    }

    if ( carries_relic(victim) )
	return FALSE;
    
    /* safe room? */
    if ( !theory && !ignore_safe && IS_SET(victim->in_room->room_flags,ROOM_SAFE) )
    {
	if ( !quiet )
	    send_to_char("Not in this room.\n\r",ch);
        return TRUE;
    }

    /* safe char? */
    if ( IS_NPC(victim) && IS_SET(victim->act, ACT_SAFE) )
    {
	if ( !quiet )
	    act( "$N is safe from your attacks.", ch, NULL, victim, TO_CHAR );
        return TRUE;
    }

    /* warfare */
    if ( !theory && PLR_ACT(ch, PLR_WAR) && PLR_ACT(victim, PLR_WAR) )
	return FALSE;

    /* arena rooms */
    if ( !theory && IS_SET(victim->in_room->room_flags, ROOM_ARENA) )
	return FALSE;

    /*  Just logged in?  The ONLY permitted attacks are vs relic-holders, in arena rooms,
	or warfare battles (all of which are handled in above checks).   */
    if ( !theory && ch->pcdata != NULL && victim->pcdata != NULL
	 && ch->pcdata->pkill_timer < 0 )
    {
	if ( !quiet )
	    send_to_char( "You are too dazed from your recent jump into this dimension.\n\r", ch );
	return TRUE;
    }

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	/* wizi mobs should be safe from mortals */
	if ( IS_SET(victim->act, ACT_WIZI) )
	    return TRUE;

        if (victim->pIndexData->pShop != NULL)
        {
	    if ( !quiet )
		send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
            return TRUE;
        }
        
        /* no killing healers, trainers, etc */
        if (IS_SET(victim->act,ACT_TRAIN)
            ||  IS_SET(victim->act,ACT_PRACTICE)
            ||  IS_SET(victim->act,ACT_IS_HEALER)
            ||  IS_SET(victim->act,ACT_SPELLUP)
            ||  IS_SET(victim->act,ACT_IS_CHANGER))
        {
	    if ( !quiet )
		send_to_char("I don't think the gods would approve.\n\r",ch);
            return TRUE;
        }
        
        if ( !IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) )
        {
            if( check_kill_steal(ch,victim) )
            {
                if ( !quiet )
                    send_to_char("Kill stealing is not permitted!!!\n\r", ch );
                return TRUE;
            }

           /* no pets */
            if (IS_SET(victim->act,ACT_PET))
            {
		if ( !quiet )
		    act("But $N looks so cute and cuddly...",ch,NULL,victim,TO_CHAR);
                return TRUE;
            }
            
            /* no charmed creatures unless owner */
            if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master))
            {
		if ( !quiet )
                send_to_char("You don't own that monster.\n\r",ch);
                return TRUE;
            }
        }
        else
        {
            /* area effect spells do not hit other mobs */
            if (area && !is_same_group(victim,ch->fighting))
                return TRUE;
        }
    }
    /* killing players */
    else
    {
	/*
        if ( IS_IMMORTAL(victim) && victim->level > LEVEL_IMMORTAL)
            return TRUE;
	*/
        
        /* NPC doing the killing */
        if (IS_NPC(ch))
        {
            /* charmed mobs and pets cannot attack players while owned */
            if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
                && ch->master->fighting != victim)
            {
		if ( !quiet )
		    send_to_char("Players are your friends!\n\r",ch);
                return TRUE;
            }
            
            /* legal kill? -- mobs only hit players grouped with opponent */
            if (area && ch->fighting != NULL && !is_same_group(ch->fighting,victim))
                return TRUE;
        }
        
        /* player doing the killing */
        else
        {
            bool clanwar_valid;
            int level_offset = 5;
            
	    clanwar_valid = is_clanwar_opp(ch, victim);
	/*	|| is_religion_opp(ch, victim); */

            if (!theory && IS_TAG(ch))
            {
		if ( !quiet )
		    send_to_char("You cannot fight while playing Freeze Tag.\n\r",ch);
		return TRUE;
	    }
            
	    if( IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim )
	    {
		if( !quiet )
		    act( "But $N is your beloved master!",ch,NULL,victim,TO_CHAR );
		return TRUE;
	    }

	    /* hardcore pkillers know no level restrictions -- bad idea!!!
	    if ( (IS_SET(victim->act, PLR_HARDCORE) && IS_SET(ch->act, PLR_HARDCORE))
		 || (IS_SET(victim->act, PLR_RP) && IS_SET(ch->act, PLR_RP)) )
		return FALSE;
	    removed July 2003 */

	    if ( !clanwar_valid )
	    {
		if (!IS_SET(ch->act, PLR_PERM_PKILL))
		{
		    if ( !quiet )
			send_to_char("You are not a player killer.\n\r",ch);
		    return TRUE;
		}
            
		if (!IS_SET(victim->act, PLR_PERM_PKILL))
		{
		    if ( !quiet )
			send_to_char("That player is not a pkiller.\n\r",ch);
		    return TRUE;
		}
	    }
            
	    /* same clan but different religion => can fight
            if (is_same_clan(ch, victim))
            {
		if ( !quiet )
		    printf_to_char(ch, "%s would frown upon that.\n\r",
				   clan_table[ch->clan].patron);
		return TRUE;
	    }
	    */
            
            if (IS_SET(victim->act,PLR_KILLER))
                level_offset += 2;
            
            if (IS_SET(victim->act, PLR_THIEF))
                level_offset += 3;
            
            if (ch->level > victim->level + level_offset)
            {
		if ( !quiet )
		    send_to_char("Pick on someone your own size.\n\r",ch);
		return TRUE;
	    } 
            
            /* This was added to curb the ankle-biters. Rim 3/15/98 */
            level_offset = 5;
            
            if (IS_SET(ch->act,PLR_KILLER))
                level_offset += 2;
            
            if (IS_SET(ch->act, PLR_THIEF))
                level_offset += 3;

            if (victim->level > ch->level + level_offset)
            {
		if ( !quiet )
		    send_to_char("You might get squashed.\n\r",ch);
		return TRUE;
	    } 
        }
    }
    return FALSE;
}

bool check_kill_steal( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    bool ignore_safe =
      NPC_ACT(ch, ACT_IGNORE_SAFE) || NPC_ACT(victim, ACT_IGNORE_SAFE);

    if( IS_NPC(victim) && victim->fighting != NULL
	&& !is_same_group(ch,victim->fighting)
	&& !ignore_safe )
    {
	/* This check cycles through the PCs in the room, and ensures
	 * that none of them are 'involved' with the intended target. */
	for( vch = ch->in_room->people; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( !IS_NPC(vch)
		 && is_safe_check(ch, vch, FALSE, TRUE, FALSE)
		 && is_same_group(vch, victim->fighting) )
	      return TRUE;
	}
    }
    return FALSE;  /* The attack is not considered kill-stealing. */
}

bool is_opponent( CHAR_DATA *ch, CHAR_DATA *victim )
{
    return is_same_group(ch->fighting, victim)
	|| is_same_group(ch, victim->fighting);
}

/* get the ultimate master of a charmed char */
CHAR_DATA* get_final_master( CHAR_DATA *ch )
{
    while ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
        ch = ch->master;
    return ch;
}

/*
* See if an attack justifies a KILLER flag.
*/
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];

    /* Better safe than sorry! */
    if ( victim->in_room == NULL )
	return;

    /* no killer stuff in warfare.. including timer */
    if ( PLR_ACT(ch, PLR_WAR) || PLR_ACT(victim, PLR_WAR) )
	return;

    /* Even if a killer flag is not given, the pkill timer MUST be activated */
    if ( !IS_NPC(ch) && !IS_NPC(victim) && ch != victim )
    {
	if( ch->pcdata != NULL )
	    ch->pcdata->pkill_timer = UMAX( ch->pcdata->pkill_timer, 5*PULSE_VIOLENCE );
	if( victim->pcdata != NULL )
	    victim->pcdata->pkill_timer = UMAX( victim->pcdata->pkill_timer, 5*PULSE_VIOLENCE );
    }

    /* Out of law region -> no killer flags dispensed */
    if ( !IS_SET(victim->in_room->room_flags, ROOM_LAW) )
	return;

    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    victim = get_final_master( victim );
    /* charmed char aren't responsible for their actions, their master is */
    ch = get_final_master( ch );
    
   /*
    * NPC's are fair game.
    * So are killers and thieves.
    */
    if ( IS_NPC(victim)
	 ||   IS_SET(victim->act, PLR_KILLER)
	 ||   IS_SET(victim->act, PLR_THIEF)
	 ||   IS_SET(victim->act, PLR_WAR ))
	return;
    
    /* if they mimic an NPC, they take the consequences.. */
    if ( is_mimic(victim) )
	return;

   /*
    * Charm-o-rama.
    */
    /* removed by Bobble 9/2001 -- screwed mob usage in pkill
    if ( IS_SET(ch->affect_field, AFF_CHARM) )
    {
        if ( ch->master == NULL )
        {
            char buf[MAX_STRING_LENGTH];
            
            sprintf( buf, "Check_killer: %s bad AFF_CHARM",
                IS_NPC(ch) ? ch->short_descr : ch->name );
            bug( buf, 0 );
            affect_strip( ch, gsn_charm_person );
            REMOVE_BIT( ch->affect_field, AFF_CHARM );
            return;
        }
        
        stop_follower( ch );
        return;
    }
    */
    
   /*
    * NPC's are cool of course (as long as not charmed).
    * Hitting yourself is cool too (bleeding).
    * So is being immortal (Alander's idea).
    * And current killers stay as they are.
    */
    if ( IS_NPC(ch)
        ||   ch == victim
        ||   ch->level >= LEVEL_IMMORTAL
        ||   IS_SET(ch->act, PLR_KILLER) 
        ||   victim->level >= LEVEL_IMMORTAL)
        return;
    
    if ( !IS_SET( victim->act, PLR_WAR ) )   
    {
        SET_BIT(ch->act, PLR_KILLER);
        
        sprintf(buf, "%s has engaged %s in mortal combat!", ch->name, victim->name);
        info_message(ch, buf, TRUE);
        sprintf(buf, "%s is now a KILLER!", ch->name);
        info_message(ch, buf, TRUE);
        
        sprintf(buf,"$N is attempting to murder %s",victim->name);
        wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
    }
    
    return;
    
}

/* returns wether the character gets a penalty for fighting blind */
bool blind_penalty( CHAR_DATA *ch )
{
    int skill = get_skill( ch, gsn_blindfighting );
    if ( number_percent() < skill/2 )
    {
	check_improve( ch, gsn_blindfighting, TRUE, 15 );
	return FALSE;
    }
    return TRUE;
}

/* checks for dodge, parry, etc. */
bool check_avoid_hit( CHAR_DATA *ch, CHAR_DATA *victim, bool show )
{
    bool finesse, autohit;
    int stance = ch->stance;
    int vstance = victim->stance;
    int sector = ch->in_room->sector_type;

    if ( ch == victim )
	return FALSE;
    
    /* only normal attacks can be faded, no spells */
    if ( check_fade( ch, victim, show ) )
	return TRUE;
    if ( check_mirror( ch, victim, show ) )
	return TRUE;
    if ( check_phantasmal( ch, victim, show ) )
	return TRUE;
    if ( IS_AFFECTED(victim, AFF_FLEE) )
	return FALSE;
    
    /* chance for dodge, parry etc. ? */
    autohit =
	vstance == STANCE_KAMIKAZE
	|| vstance == STANCE_BLOODBATH;

    finesse =
	stance == STANCE_EAGLE 
	|| stance == STANCE_LION 
	|| stance == STANCE_FINESSE
	|| stance == STANCE_AMBUSH
	|| stance == STANCE_BLADE_DANCE
	|| stance == STANCE_BLOODBATH
	|| stance == STANCE_TARGET_PRACTICE
	|| stance == STANCE_KAMIKAZE
        || stance == STANCE_SERPENT;

    /* woodland combat */
    if ( sector == SECT_FOREST 
	 || ((sector == SECT_FIELD 
	      || sector == SECT_HILLS 
	      || sector == SECT_MOUNTAIN)
	     && number_bits(1) == 0) )
    {
	if ( number_percent() <= get_skill(ch, gsn_woodland_combat) )
	{
	    finesse = TRUE;
	    check_improve( ch, gsn_woodland_combat, TRUE, 10 );
	}
	else
	    check_improve( ch, gsn_woodland_combat, FALSE, 10 );
    }

    if ( !autohit && (vstance == STANCE_BUNNY || !(finesse && number_bits(1) == 0)) )
    {
	if ( check_outmaneuver( ch, victim ) )
	    return TRUE;
        if ( check_duck( ch,victim ) )
            return TRUE;
        if ( check_dodge( ch, victim ) )
            return TRUE;
        if ( check_parry( ch, victim ) )
            return TRUE;
    }
    
    if ( check_shield_block(ch,victim) )
	return TRUE;
    
    return FALSE;
}

bool check_fade( CHAR_DATA *ch, CHAR_DATA *victim, bool show ) 
{
    bool ch_fade, victim_fade;
    int chance;

    /* don't fade own attacks */
    if ( victim == ch )
	return FALSE;

    /* victim */
    if ( ch->stance == STANCE_DIMENSIONAL_BLADE )
	chance = 0;
    else if ( victim->stance == STANCE_SHADOWWALK )
	chance = 50;
    else if ( IS_AFFECTED(victim, AFF_FADE) 
	      || IS_AFFECTED(victim, AFF_CHAOS_FADE)
	      || (IS_NPC(victim) && IS_SET(victim->off_flags, OFF_FADE)) )
	chance = 30;
    else if ( IS_AFFECTED(victim, AFF_MINOR_FADE ) )
        chance = 15;
    else
	chance = 0;

    victim_fade = ( number_percent() <= chance );

    /* attacker */
    if ( IS_AFFECTED(ch, AFF_CHAOS_FADE)
	 && !(ch->stance == STANCE_SHADOWWALK
	      || IS_AFFECTED(ch, AFF_FADE)
	      || (IS_NPC(ch) && IS_SET(ch->off_flags, OFF_FADE))) )
	chance = 15;
    else
	chance = 0;
    
    ch_fade = ( number_percent() <= chance );

    /* if none or both fade it's a hit */
    if ( ch_fade == victim_fade )
	return FALSE;

    /* otherwise one fades */
    if ( !show )
	return TRUE;

    /* now let's see who.. */
    if ( victim_fade )
    {
        act_gag( "$n's attack passes harmlessly through you.", 
		 ch, NULL, victim, TO_VICT, GAG_FADE );
        act_gag( "$N fades through your attack.",
		 ch, NULL, victim, TO_CHAR, GAG_FADE );
        act_gag( "$N fades through $n's attack.",
		 ch, NULL, victim, TO_NOTVICT, GAG_FADE );
    }
    else
    {
        act_gag( "You fade on your own attack!",
		 ch, NULL, victim, TO_CHAR, GAG_FADE );
        act_gag( "$n fades on $s own attack.", 
		 ch, NULL, victim, TO_ROOM, GAG_FADE );
    }
    
    return TRUE;
}

bool check_mirror( CHAR_DATA *ch, CHAR_DATA *victim, bool show ) 
{
    AFFECT_DATA *aff;
    
    if ( (aff = affect_find(victim->affected, gsn_mirror_image)) == NULL )
	return FALSE;

    if ( number_range(0, aff->bitvector) == 0
	 || chance(get_skill(ch, gsn_alertness)/2) )
	return FALSE;

    /* ok, we hit a mirror image */
    if ( show )
    {
        act_gag( "$n's attack hits one of your mirror images.", 
		 ch, NULL, victim, TO_VICT, GAG_FADE );
        act_gag( "You hit one of $N's mirror images, which dissolves.",
		 ch, NULL, victim, TO_CHAR, GAG_FADE );
        act_gag( "$n hits one of $N's mirror images, which dissolves.",
		 ch, NULL, victim, TO_NOTVICT, GAG_FADE );
    }

    /* lower number of images */
    aff->bitvector--;
    if ( aff->bitvector <= 0 )
    {
	send_to_char( "Your last mirror image has dissolved!\n\r", victim );
	affect_strip( victim, gsn_mirror_image );
    }

    return TRUE;
}

bool check_phantasmal( CHAR_DATA *ch, CHAR_DATA *victim, bool show ) 
{
    int dam;
    AFFECT_DATA *aff;
    
    if ( (aff = affect_find(victim->affected, gsn_phantasmal_image)) == NULL )
	return FALSE;

    if ( number_range(0, aff->bitvector) == 0
	 || chance(get_skill(ch, gsn_alertness)/2) )
	return FALSE;

    /* ok, we hit a phantasmal image */
    if ( show )
    {
        act_gag( "$n's attack hits one of your phantasmal images.", 
		 ch, NULL, victim, TO_VICT, GAG_FADE );
        act_gag( "You hit one of $N's phantasmal images, which dissolves.",
		 ch, NULL, victim, TO_CHAR, GAG_FADE );
        act_gag( "$n hits one of $N's phantasmal images, which dissolves.",
		 ch, NULL, victim, TO_NOTVICT, GAG_FADE );

	dam = dice(11, 7);
	full_dam(victim, ch, dam, gsn_phantasmal_image, DAM_FIRE, TRUE);
	        
    }

    /* lower number of images */
    aff->bitvector--;
    if ( aff->bitvector <= 0 )
    {
	send_to_char( "Your last phantasmal image has dissolved!\n\r", victim );
	affect_strip( victim, gsn_phantasmal_image );
    }

    return TRUE;
}

/*
* Check for parry.
*/
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;
    int ch_weapon, victim_weapon;
    OBJ_DATA *ch_weapon_obj = NULL,
	*victim_weapon_obj = NULL;

    if ( !IS_AWAKE(victim) )
        return FALSE;
    
    ch_weapon = get_weapon_sn(ch);
    victim_weapon = get_weapon_sn(victim);

    if ( ch_weapon == gsn_gun || victim_weapon == gsn_gun
	 || ch_weapon == gsn_bow || victim_weapon == gsn_bow )
    {
        return FALSE;
    }

    chance = get_skill(victim, gsn_parry) / 4 + 10;
    chance += (get_curr_stat(victim, STAT_DEX) - get_curr_stat(ch, STAT_DEX)) / 8;
    
    if (get_skill(victim, gsn_parry) == 100)
            chance += 2;
    
    /* some weapons are better for parrying, some are worse.. */
    if ( victim_weapon == gsn_hand_to_hand )
    {
        if ( !IS_NPC(victim) || !IS_SET(victim->off_flags, OFF_PARRY) )
            return FALSE;
    }
    else if ( victim_weapon == gsn_sword )
	chance += 5;
    else if ( victim_weapon == gsn_flail )
	chance -= 10;
    else if ( victim_weapon == gsn_whip )
	chance -= 10;

    /* ..and some weapons are harder to parry */
    if ( ch_weapon == gsn_whip || ch_weapon == gsn_flail )
	chance -= 10;
    else if ( ch_weapon == gsn_hand_to_hand )
	chance -= 5;

    /* two-handed weapons are harder to parry with non-twohanded */
    if ( (ch_weapon_obj = get_eq_char(ch, WEAR_WIELD)) != NULL
	 && IS_WEAPON_STAT(ch_weapon_obj, WEAPON_TWO_HANDS) )
    {
	if ( (victim_weapon_obj = get_eq_char(victim, WEAR_WIELD)) == NULL
	     || !IS_WEAPON_STAT(victim_weapon_obj, WEAPON_TWO_HANDS) )
	    chance /= 2;

        if (victim->stance == STANCE_AVERSION)
            chance += 10; 
    }

    if (victim->stance == STANCE_SWAYDES_MERCY)
        chance += 5 + get_skill( ch, gsn_swaydes_mercy )/10;

    if (victim->stance == STANCE_AVERSION)
        chance += 10; 

    if (!can_see(ch,victim) && blind_penalty(victim))
        chance /= 2;
    
    if ( IS_AFFECTED(victim, AFF_SORE) )
	chance -= 10;
    
    if ( number_percent( ) >= chance + (victim->level - ch->level)/4 )
        return FALSE;
    
    act_gag( "You parry $n's attack.",  ch, NULL, victim, TO_VICT, GAG_MISS );
    act_gag( "$N parries your attack.", ch, NULL, victim, TO_CHAR, GAG_MISS );
    act_gag( "$N parries $n's attack.", ch, NULL, victim, TO_NOTVICT, GAG_MISS );
    check_improve(victim,gsn_parry,TRUE,15);

    /* whips can disarm or get disarmed on successfull parry */
    if ( ch_weapon == gsn_whip && number_bits(5) == 0 )
    {
	if ( victim_weapon != gsn_hand_to_hand && number_bits(1) )
	{
	    /* disarm */
	    act( "Your whip winds around $N's weapon.", ch, NULL, victim, TO_CHAR );
	    act( "$n's whip winds around your weapon.", ch, NULL, victim, TO_VICT );
	    act( "$n's whip winds around $N's weapon.", ch, NULL, victim, TO_NOTVICT );
	    disarm( ch, victim, FALSE );
	}
	else
	{
	    /* get disarmed */
	    act( "Your whip winds around $N's arm.", ch, NULL, victim, TO_CHAR );
	    act( "$n's whip winds around your arm.", ch, NULL, victim, TO_VICT );
	    act( "$n's whip winds around $N's arm.", ch, NULL, victim, TO_NOTVICT );
	    disarm( victim, ch, FALSE );
	}
    }
    else
    {
	OBJ_DATA *wield;
	int dam, dam_type;
	/* parrying a hand-to-hand attack with a weapon deals damage */
	if ( ch_weapon == gsn_hand_to_hand
	     && !IS_NPC(ch)
	     && (wield = get_eq_char(victim, WEAR_WIELD)) != NULL )
	{
	    dam = one_hit_damage( victim, gsn_parry, wield );
	    dam_type = get_weapon_damtype( wield );
	    full_dam( victim, ch, dam, gsn_parry, dam_type, TRUE );
	}
    }

    return TRUE;
}

/*
* Check for duck!
*/
bool check_duck( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;
    
    if ( !IS_AWAKE(victim) )
        return FALSE;
    
    if ( get_weapon_sn(ch) != gsn_gun
	 && get_weapon_sn(ch) != gsn_bow )
        return FALSE;
    
    chance = get_skill(victim,gsn_duck) / 3;
    
    if (chance == 0)
	return FALSE;
    
    if (victim->stance==STANCE_SHOWDOWN)
        chance += 30;

    if ( IS_AFFECTED(victim, AFF_SORE) )
	chance -= 10;
    
    if (!can_see(victim,ch) && blind_penalty(victim))
        chance -= chance / 4;

    if (get_skill(victim,gsn_duck) == 100)
        chance += 5;
    
    if ( number_percent( ) >= chance + (victim->level - ch->level)/4 )
        return FALSE;
    
    act_gag( "You duck $n's shot!", ch, NULL, victim, TO_VICT, GAG_MISS );
    act_gag( "$N ducks your shot!", ch, NULL, victim, TO_CHAR, GAG_MISS );
    act_gag( "$N ducks $n's shot.", ch, NULL, victim, TO_NOTVICT, GAG_MISS );
    check_improve(victim,gsn_duck,TRUE,15);
    return TRUE;
}

bool check_outmaneuver( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
        return FALSE;
    
    if ( victim->fighting == ch || victim->fighting == NULL )
	return FALSE;

    /* can't outmaneuver ranged weapons */
    if ( get_weapon_sn(ch) == gsn_gun
	 || get_weapon_sn(ch) == gsn_bow )
        return FALSE;

    chance = get_skill(victim, gsn_mass_combat) / 3;
    
    if (chance == 0)
	return FALSE;
    
    if ( IS_AFFECTED(victim, AFF_SORE) )
	chance -= 10;
    
    if ( !can_see(victim,ch) && blind_penalty(victim) )
        chance -= chance/4;

    /* Works at 95%, instead of 100%, since 100% effective
       is near impossible - Astark Nov 2012 */
    if (get_skill(victim,gsn_mass_combat) >= 95)
        chance += 3;
    
    if ( number_percent() > chance )
        return FALSE;
    
    act_gag( "You outmaneuver $n's attack!", ch, NULL, victim, TO_VICT, GAG_MISS );
    act_gag( "$N outmaneuvers your attack!", ch, NULL, victim, TO_CHAR, GAG_MISS );
    act_gag( "$N outmaneuvers $n's attack.", ch, NULL, victim, TO_NOTVICT, GAG_MISS );
    check_improve(victim,gsn_mass_combat,TRUE,15);
    return TRUE;
}

bool check_jam( CHAR_DATA *ch, int odds, bool both )
{
    OBJ_DATA *first;
    OBJ_DATA *second;
    
    if ( ch->stance == STANCE_SHOWDOWN && number_bits(2) )
	return FALSE;

    if ( odds >= number_range(1, 1000) ) 
    {
	if( get_weapon_sn_new(ch,FALSE) != gsn_gun )
	    return FALSE;
	if( (first = get_eq_char( ch, WEAR_WIELD )) == NULL )
	    return FALSE;
	SET_BIT(first->extra_flags,ITEM_JAMMED); 
	send_to_char( "Your gun is jammed!\n\r", ch );
	return TRUE;
    }

    if( both && odds >= number_range(1, 1000) )
    {
	if( get_weapon_sn_new(ch,TRUE) != gsn_gun )
	    return FALSE;
	if( (second = get_eq_char(ch,WEAR_SECONDARY)) == NULL )
	    return FALSE;
	SET_BIT(second->extra_flags,ITEM_JAMMED); 
	send_to_char( "Your offhand gun is jammed!\n\r", ch );
	return TRUE;   // because in semiauto and the like, second gun is not rechecked
    }
    
    return FALSE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;
    int chance;
    
    if ( !IS_AWAKE(victim) )
        return FALSE;
    
    if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
        return FALSE;

    chance = 10 + get_skill(victim, gsn_shield_block) / 4;

    if ( (obj = get_eq_char(victim, WEAR_WIELD)) &&
	 (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS) ||
	  get_eq_char(victim, WEAR_SECONDARY)))
    {
	chance = chance * (100 + get_skill(victim, gsn_wrist_shield)) / 300;
	check_improve(victim, gsn_wrist_shield, TRUE, 20);
    }

    /* whips are harder to block */
    if ( get_weapon_sn(ch) == gsn_whip )
	chance -= 10;
    
    if (victim->stance == STANCE_SWAYDES_MERCY)
        chance += 5 + get_skill( ch, gsn_swaydes_mercy )/10; 

    if (victim->stance == STANCE_AVERSION)
        chance += 10;
    
    if ( !can_see(victim,ch) && blind_penalty(victim) )
        chance -= chance / 4;

    if ( IS_AFFECTED(victim, AFF_SORE) )
	chance -= 10;

    if (get_skill(victim, gsn_shield_block) == 100)
        chance += 2;

    if ( number_percent( ) >= chance + (victim->level - ch->level)/4 )
        return FALSE;
    
    act_gag( "You block $n's attack with your shield.",  ch, NULL, victim, 
        TO_VICT, GAG_MISS );
    act_gag( "$N blocks your attack with a shield.", ch, NULL, victim, 
        TO_CHAR, GAG_MISS );
    act_gag( "$N blocks $n's attack with $S shield.", ch, NULL, victim, 
        TO_NOTVICT, GAG_MISS );
    check_improve(victim,gsn_shield_block,TRUE,15);
    return TRUE;
}


/*
* Check for dodge.
*/
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;
    bool evade = FALSE;
    
    if ( !IS_AWAKE(victim) )
        return FALSE;
    
    chance = get_skill(victim,gsn_dodge) / 4 + 10;
    chance += (get_curr_stat(victim, STAT_AGI)-get_curr_stat(ch, STAT_DEX))/8;

    if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
        evade = TRUE;
        chance += get_skill(victim, gsn_evasive) / 4;
    }

    if (!can_see(victim,ch) && blind_penalty(victim))
        chance -= chance / 4;
    
    if ( victim->stance==STANCE_TOAD
	 || victim->stance==STANCE_SWAYDES_MERCY
	 || victim->stance==STANCE_BUNNY
	 || IS_SET(victim->form, FORM_DOUBLE_JOINTED) )
        chance += 15;

    if ( victim->stance==STANCE_AVERSION)
        chance += 7;

    if ( get_skill(ch, gsn_dodge) == 100)
        chance += 2;
    
    if ( IS_AFFECTED(victim, AFF_SORE) )
	chance -= 10;

    if ( number_percent( ) >= UMIN(chance + (victim->level - ch->level)/4,90) )
        return FALSE;
    
    act_gag( "You dodge $n's attack.", ch, NULL, victim, TO_VICT, GAG_MISS );
    act_gag( "$N dodges your attack.", ch, NULL, victim, TO_CHAR, GAG_MISS );
    act_gag( "$N dodges $n's attack.", ch, NULL, victim, TO_NOTVICT, GAG_MISS );
    check_improve(victim,gsn_dodge,TRUE,15);
    if (evade)
      check_improve(victim,gsn_evasive,TRUE,15);

    return TRUE;
}




/*
 * Change a victim's position (during combat)
 */
void set_pos( CHAR_DATA *ch, int position )
{
    if ( ch->position == position )
	return;

    ch->position = position;
    ch->on = NULL;
}

/*
* Set correct position of a victim.
*/
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
        if ( victim->position <= POS_STUNNED )
	    if ( victim->fighting != NULL )
		set_pos( victim, POS_FIGHTING );
	    else
		set_pos( victim, POS_STANDING );
	/* make sure fighters wake up */
	if ( victim->fighting != NULL && victim->position == POS_SLEEPING )
	    set_pos( victim, POS_RESTING );
        return;
    }
    
    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	set_pos( victim, POS_DEAD );
        return;
    }
    
    if ( victim->hit <= -11 )
    {
	set_pos( victim, POS_DEAD );
        return;
    }
    
    if ( victim->hit <= -6 ) 
	set_pos( victim, POS_MORTAL );
    else if ( victim->hit <= -3 ) 
	set_pos( victim, POS_INCAP );
    else
	set_pos( victim, POS_STUNNED );
    
    return;
}

/*
* Start fights.
*/
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    set_fighting_new( ch, victim, TRUE );
}

void set_fighting_new( CHAR_DATA *ch, CHAR_DATA *victim, bool kill_trigger )
{
    /*
    if ( ch->fighting != NULL )
    {
        //  bug( "Set_fighting: already fighting", 0 );
        ch->fighting = victim;
        return;
    }
    */

    if ( IS_AFFECTED( ch, AFF_OVERCHARGE))
    {
	affect_strip_flag( ch, AFF_OVERCHARGE );
        send_to_char( "Your mana calms down as you refocus and ready for battle.\n\r", ch );
    }

    if (victim && IS_NPC(ch) && IS_SET(ch->off_flags, OFF_HUNT))
	set_hunting(ch, victim);
    
    if ( IS_AFFECTED(ch, AFF_SLEEP) )
        affect_strip_flag( ch, AFF_SLEEP );
    if ( ch->position == POS_SLEEPING )
	set_pos( ch, POS_RESTING );
    
    ch->fighting = victim;

    if ( kill_trigger && check_kill_trigger(ch, victim) )
	return;

    if ( ch->position >= POS_FIGHTING )
      set_pos( ch, POS_FIGHTING );
}

/*
 * check for kill trigger - returns wether attack was canceled
 */
bool check_kill_trigger( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *old_victim = ch->fighting;

    ch->fighting = victim;
    if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
	mp_percent_trigger( victim, ch, NULL, NULL, TRIG_KILL );
    if ( ch->fighting != victim )
	return TRUE;

    ch->fighting = old_victim;
    return FALSE;
}

/*
* Stop fights.
*/
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;
    
    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
        if ( fch == ch || ( fBoth && fch->fighting == ch ) )
        {
            fch->fighting   = NULL;
	    if ( IS_NPC(fch) && !IS_AFFECTED(fch, AFF_CHARM) )
		set_pos( fch, fch->default_pos );
	    else
		set_pos( fch, POS_STANDING );
            
            update_pos( fch );
        }
    }
    
    return;
}

/* returns wether an attack should be canceled */
bool stop_attack( CHAR_DATA *ch, CHAR_DATA *victim )
{
    return ch == NULL
	|| victim == NULL
	|| ch->in_room == NULL
	|| victim->in_room == NULL
	|| ch->in_room != victim->in_room
	|| IS_DEAD(ch)
	|| IS_DEAD(victim);
}

/*
void extract_sticky_to_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    OBJ_DATA *in, *in_next;

    if ( obj->contains == NULL )
	return;

    for (in = obj->contains; in != NULL; in = in_next)
    {
	in_next = in->next_content;
	if ( IS_SET(in->extra_flags, ITEM_STICKY) )
	{
	    obj_from_obj(in);
	    obj_to_char(in, ch);
	}
	else
	    extract_sticky_to_char( ch, in );
    }
}
*/

/*
* Make a corpse out of a character.
*/
void make_corpse( CHAR_DATA *victim, CHAR_DATA *killer, bool go_morgue)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    ROOM_INDEX_DATA *location;
    bool eqloot = TRUE;
    
    if (go_morgue)
        location = get_room_index ( ROOM_VNUM_MORGUE );
    else
        location = victim->in_room;
    
    if ( IS_NPC(victim) )  /* MOB Death */
    {
        name        = victim->short_descr;
        corpse      = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
        corpse->timer   = number_range( 25, 40 );
        
        if (killer && !IS_NPC(killer) && !go_morgue)
            corpse->owner = str_dup(killer->name);
        
        if ( victim->gold > 0 || victim->silver > 0 )
        {

         /* Added a check for the fortune bit. This is assigned by the new god_fortune
            blessing, and increases gold/silver drops by 50 percent - Astark 12-23-12 */

         /* This was causing a crash from commands like slay, that have a null killer
            value. Fixed by adding a killer null check. 1-8-13 - Astark */
            if (killer != NULL)
            {
                if (IS_AFFECTED(killer, AFF_FORTUNE))
                {
                    obj_to_obj( create_money( victim->gold*3/2, victim->silver*3/2 ), corpse );
                    victim->gold = 0;
                    victim->silver = 0;
                }
                else
                {
                    obj_to_obj( create_money( victim->gold, victim->silver ), corpse );
                    victim->gold = 0;
                    victim->silver = 0;
                }
            }
        }
        corpse->cost = 0;
    }
    else  /* Player death */
    {
        name        = victim->name;
        corpse      = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
        corpse->timer   = number_range( 25, 40 );
        
        REMOVE_BIT(victim->act, PLR_CANLOOT);
	victim->stance = 0;
        
        /* If dead player is not a pkiller, he will own his corpse.
        Otherwise, the victor will own the corpse and may loot it. */
        /*	  if (!IS_SET(victim->act, PLR_PERM_PKILL))
        corpse->owner = str_dup(victim->name);
        else
        { If the player dies from a pkiller, they should be in clanwar*/


        if (killer && !IS_NPC(killer))
	{
            corpse->owner = str_dup(killer->name);
	    /* This is the only place where eqloot could be set to FALSE..... */
	    /* And then, only if both players are not HC, or both are not RP. */
	    eqloot = ( IS_SET(victim->act,PLR_HARDCORE) && IS_SET(killer->act,PLR_HARDCORE) )
		     ||	( IS_SET(victim->act,PLR_RP) && IS_SET(killer->act,PLR_RP) );
	}
        else
            corpse->owner = str_dup(victim->name);

        if (victim->gold > 1 || victim->silver > 1)
        {
            obj_to_obj(create_money(victim->gold / 2, victim->silver/2), corpse);
            victim->gold -= victim->gold/2;
            victim->silver -= victim->silver/2;
        }
        /*}*/
        
        corpse->cost = 0;
    }
    
    corpse->level = victim->level;
    
    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );
    
    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );
    
    /*
    if ( !IS_NPC(victim) )
	extract_remort_eq( victim );
    */
    if ( !IS_NPC(victim) )
	extract_char_eq( victim, &is_remort_obj, -1 );
    extract_char_eq( victim, &is_drop_obj, TO_ROOM );

    for ( obj = victim->carrying; obj != NULL; obj = obj_next )
    {
        bool floating = FALSE;

        obj_next = obj->next_content;
        
        if (IS_SET(obj->extra_flags, ITEM_STICKY))
        {
            if (IS_NPC(victim))
            {
                if (obj->owner == NULL && killer)
                    obj->owner = str_dup(killer->name);
            }
            else
                continue;
        }
        
        if ( obj->wear_loc == WEAR_FLOAT )
            floating = TRUE;
        if (obj->item_type == ITEM_POTION)
            obj->timer = number_range(500,1000);
        if (obj->item_type == ITEM_SCROLL)
            obj->timer = number_range(1000,2500);
        if ( IS_SET(obj->extra_flags, ITEM_ROT_DEATH) )
        {
            obj->timer = number_range(5,10);
	    /*
            REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
	    */
        }
        REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
        
        
        /* not all items shall be lost */
	/* If the p vs p fight was not with eqlooting, only easy_drop items fall */
        if (!IS_NPC(victim)
	    && !IS_OBJ_STAT(obj, ITEM_EASY_DROP)
	    && (number_percent() < 75))
            continue;
	else if ( !IS_NPC(victim) && !eqloot )
	    continue;
        
        /* Logs all EQ that goes to corpses for easier and more accurate
           reimbursing. - Astark Oct 2012 */

        if (!IS_NPC(victim))
        {
            sprintf( buf, "%s died in room %d. EQ To Corpse = %d", victim->name, 
                victim->in_room->vnum, obj->pIndexData->vnum );
    	    log_string( buf );
        }

	/* extract sticky eq from of container */
	if ( !IS_NPC(victim) )
	    extract_char_obj( victim, &is_sticky_obj, TO_CHAR, obj );

        obj_from_char( obj );

        if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
            extract_obj( obj );  
        else if (floating)
        {
            if ( IS_OBJ_STAT(obj,ITEM_ROT_DEATH) ) /* get rid of it */
            { 
                if (obj->contains != NULL)
                {
                    OBJ_DATA *in, *in_next;
                    
                    act("$p evaporates,scattering its contents.",
                        victim,obj,NULL,TO_ROOM);
                    for (in = obj->contains; in != NULL; in = in_next)
                    {
                        in_next = in->next_content;
                        obj_from_obj(in);
                        obj_to_room(in,victim->in_room);
                    }
                }
                else
                    act("$p evaporates.",
                    victim,obj,NULL,TO_ROOM);
                extract_obj(obj);
            }
            else
            {
                act("$p falls to the floor.",victim,obj,NULL,TO_ROOM);
                obj_to_room(obj,victim->in_room);
             }
        }
        else
        {
            obj_to_obj( obj, corpse );
        }
    }
    
    if ( IS_NPC(victim) )
    {
        obj_to_room( corpse,victim->in_room );
    }
    else
        obj_to_room( corpse,location );
    
    return;
}


/*
* Improved Death_cry contributed by Diavolo.
*/
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;
    
    vnum = 0;
    msg = "You hear $n's death cry.";
    
    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";         break;
    case  1: 
        if (ch->material == 0)
        {
            msg  = "$n splatters blood on your armor.";     
            break;
        }
    case  2:                            
        if (IS_SET(ch->parts,PART_GUTS))
        {
            msg = "$n spills $s guts all over the floor.";
            vnum = OBJ_VNUM_GUTS;
        }
        break;
    case  3: 
        if (IS_SET(ch->parts,PART_HEAD))
        {
            msg  = "$n's severed head plops on the ground.";
            vnum = OBJ_VNUM_SEVERED_HEAD;               
        }
        break;
    case  4: 
        if (IS_SET(ch->parts,PART_HEART))
        {
            msg  = "$n's heart is torn from $s chest.";
            vnum = OBJ_VNUM_TORN_HEART;             
        }
        break;
    case  5: 
        if (IS_SET(ch->parts,PART_ARMS))
        {
            msg  = "$n's arm is sliced from $s dead body.";
            vnum = OBJ_VNUM_SLICED_ARM;             
        }
        break;
    case  6: 
        if (IS_SET(ch->parts,PART_LEGS))
        {
            msg  = "$n's leg is sliced from $s dead body.";
            vnum = OBJ_VNUM_SLICED_LEG;             
        }
        break;
    case 7:
        if (IS_SET(ch->parts,PART_BRAINS))
        {
            msg = "$n's head is shattered, and $s brains splash all over you.";
            vnum = OBJ_VNUM_BRAINS;
        }
    }
    
    act( msg, ch, NULL, NULL, TO_ROOM );
    
    if ( vnum != 0 )
    {
        char buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        char *name;
        
        name        = IS_NPC(ch) ? ch->short_descr : ch->name;
        obj     = create_object( get_obj_index( vnum ), 0 );
        obj->timer  = number_range( 4, 7 );
        
        sprintf( buf, obj->short_descr, name );
        free_string( obj->short_descr );
        obj->short_descr = str_dup( buf );
        
        sprintf( buf, obj->description, name );
        free_string( obj->description );
        obj->description = str_dup( buf );
        
        if (obj->item_type == ITEM_FOOD)
        {
            if (IS_SET(ch->form,FORM_POISON))
                obj->value[3] = 1;
            else if (!IS_SET(ch->form,FORM_EDIBLE))
                obj->item_type = ITEM_TRASH;
        }
        
        obj_to_room( obj, ch->in_room );
    }
    
    if ( IS_NPC(ch) )
        msg = "You hear something's death cry.";
    else
        msg = "You hear someone's death cry.";
    
    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
        EXIT_DATA *pexit;
        
        if ( ( pexit = was_in_room->exit[door] ) != NULL
            &&   pexit->u1.to_room != NULL
            &&   pexit->u1.to_room != was_in_room )
        {
            ch->in_room = pexit->u1.to_room;
            act( msg, ch, NULL, NULL, TO_ROOM );
        }
    }
    ch->in_room = was_in_room;
    
    return;
}



void raw_kill( CHAR_DATA *victim, CHAR_DATA *killer, bool to_morgue )
{
    int i;
    ROOM_INDEX_DATA *kill_room = victim->in_room;
    
    /* backup in case hp goes below 1 */
    if (NOT_AUTHED(victim))
    {
        bug( "raw_kill: killing unauthed", 0 );
        return;
    }
    
    stop_fighting( victim, TRUE );
    death_cry( victim );
    death_penalty( victim );

    if ( IS_NPC(victim) && 
	 (victim->pIndexData->vnum == MOB_VNUM_VAMPIRE
	  || IS_SET(victim->form, FORM_INSTANT_DECAY)) )
    {
	act( "$n crumbles to dust.", victim, NULL, NULL, TO_ROOM );
	drop_eq( victim );
    }
    else if ( IS_NPC(victim) || !IS_SET(kill_room->room_flags, ROOM_ARENA) )
	make_corpse( victim, killer, to_morgue);
    
    if ( IS_NPC(victim) )
    {
        victim->pIndexData->killed++;
        kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
        extract_char( victim, TRUE );
	update_room_fighting( kill_room );
        return;
    }

    victim->pcdata->condition[COND_HUNGER] =
	UMAX(victim->pcdata->condition[COND_HUNGER], 20);
    victim->pcdata->condition[COND_THIRST] =
	UMAX(victim->pcdata->condition[COND_THIRST], 20);

    extract_char_new( victim, FALSE, FALSE );
    while ( victim->affected )
        affect_remove( victim, victim->affected );
    morph_update( victim );
    /* what's that supposed to do? --Bobble
    for (i = 0; i < 4; i++)
        victim->armor[i] = 100;
    */
    set_pos( victim, POS_RESTING );
    victim->hit     = UMAX( 1, victim->hit  );
    victim->mana    = UMAX( 1, victim->mana );
    victim->move    = UMAX( 1, victim->move );
    update_room_fighting( kill_room );
    return;
}

/* check if the gods have mercy on a character */
bool check_mercy( CHAR_DATA *ch )
{
    int chance = 1000;
    chance += get_curr_stat(ch, STAT_CHA) * 4 + get_curr_stat(ch, STAT_LUC);
    chance += ch->alignment;
    
    if (IS_SET(ch->act, PLR_KILLER) 
        && ch->class != class_lookup("assassin"))
        chance -= 500;
    if (IS_SET(ch->act, PLR_THIEF)
        && ch->class != class_lookup("thief"))
        chance -= 500;
    
    return number_range(0, 2999) < chance;
}


/* penalize players if they die */
void death_penalty( CHAR_DATA *ch )
{
   
    int loss_choice;
    int curr_level_exp;
    
    /* NPCs get no penalty */
    if (IS_NPC(ch))
        return;
    
    if ( ch->in_room != NULL && IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
	return;

    /* check for mercy from the gods */
    if (check_mercy(ch))
    { 
        send_to_char("The gods have mercy on your soul.\n\r", ch);
        return;
    }
    
    /* experience penalty - 2/3 way back to previous level. */
    curr_level_exp = exp_per_level(ch, ch->pcdata->points) * ch->level;
    if ( ch->exp > curr_level_exp )
        gain_exp( ch, (curr_level_exp - ch->exp) * 2/3 );
    
    /* get number of possible loss choices */
    loss_choice = 0;
    if (ch->pcdata->trained_hit > 0)
        loss_choice++;
    if (ch->pcdata->trained_mana > 0)
        loss_choice++;
    if (ch->pcdata->trained_move > 0)
        loss_choice++;
    
    /* randomly choose a stat to lose */
    if (loss_choice == 0)
        return;
    loss_choice = number_range(1, loss_choice);
    
    /* hp/mana/move loss if player trained it */
    if (ch->pcdata->trained_hit > 0 && --loss_choice == 0)
    {
        ch->pcdata->trained_hit--;
        update_perm_hp_mana_move(ch);
        send_to_char("You feel your health dwindle.\n\r", ch);
        return;
    }
    
    if (ch->pcdata->trained_mana > 0 && --loss_choice == 0)
    {
        ch->pcdata->trained_mana--;
        update_perm_hp_mana_move(ch);
        send_to_char("You feel your mental powers dwindle.\n\r", ch);
        return;
    }
    
    if (ch->pcdata->trained_move > 0)
    {
        ch->pcdata->trained_move--;
        update_perm_hp_mana_move(ch);
        send_to_char("You feel your endurance dwindle.\n\r", ch);
        return;
    }
}

void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    MEM_DATA *m;
    CHAR_DATA *gch;
    int xp;
    int members;
    int min_xp;
    int high_level, low_level;
    int high_align, low_align;
    int total_dam, group_dam;
    char buf[MSL];

    /*
     * Monsters don't get kill xp's or changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    
    if ( victim == ch || !IS_NPC(victim) )
        return;
    
    members = 0;
    /*
    high_level = ch->level;
    low_level = high_level;
    high_align = ch->alignment;
    low_align = high_align;
    */
    high_level = 1;
    low_level = 100;
    high_align = -1000;
    low_align = 1000;
    min_xp = 1000;

    total_dam=0;
    group_dam=0;

    for (m = victim->aggressors; m; m=m->next)
	total_dam += m->reaction;
    total_dam = UMAX(1, total_dam);

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( is_same_group( gch, ch ) )
        {
            members++;

            if (!IS_NPC(gch))
	    {
		xp = xp_compute(gch, victim, 0);
		min_xp = UMIN(xp, min_xp);

		high_align = UMAX(high_align, gch->alignment);
		low_align = UMIN(low_align, gch->alignment);
		high_level = UMAX(high_level, gch->level);
		low_level = UMIN(low_level, gch->level);
		for (m=victim->aggressors; m; m=m->next)
		    if (gch->id == m->id)
		    {
			group_dam += m->reaction;
			/* m->reaction = 0; */
		    }
	    }
        }
    }
    
    /* exp for chars who helped killing but left the room */
    /* nice but costs too much resources --Bobble
    for (m = victim->aggressors; m; m=m->next)
	if (m->reaction > 0)
	    for (gch = char_list; gch; gch=gch->next)
		if (!IS_NPC(gch) && (gch->id==m->id))
		{
		    if ((ch!=gch) && 
			!((ch->in_room == gch->in_room) &&
			  is_same_group(gch, ch)))
		    {
			xp = xp_compute(gch, victim, 100 * m->reaction/total_dam);
			xp = xp * m->reaction / total_dam;
			if ( is_same_player(ch, gch) )
			{
			    sprintf( buf, "Multiplay: %s gaining experience from %s's kill",
				     gch->name, ch->name );
			    wiznet(buf, ch, NULL, WIZ_CHEAT, 0, LEVEL_IMMORTAL);
			}
			else
			    gain_exp(gch, xp);
		    }
		    break;
		}
    */

    /* reduce high/low align and level range by charisma */
    {
	CHAR_DATA *leader = ch->leader ? ch->leader : ch;

        xp = (high_align-low_align)/2;
        xp = xp * get_curr_stat(leader,STAT_CHA) / 200;
        high_align-=xp;
        low_align+=xp;
        
        xp = (high_level-low_level)/2;
        xp = xp * get_curr_stat(leader,STAT_CHA) / 200;
        high_level-=xp;
        low_level+=xp;
    }
    
    if ( members == 0 )
    {
        bug( "Group_gain: members.", members );
        members = 1;
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        OBJ_DATA *obj;
        OBJ_DATA *obj_next;
	int dam_done;
        
        if ( !is_same_group(gch, ch) || IS_NPC(gch))
            continue;
        
	dam_done = get_reaction( victim, gch );
        xp = xp_compute( gch, victim, 100 * dam_done/total_dam );
	/* partly exp from own, partly from group */ 
	xp = (min_xp * group_dam + (xp - min_xp) * dam_done)/total_dam;
	xp = number_range( xp * 9/10, xp * 11/10 );
        
	/*
        if ( gch->level - low_level > 20 )
        {
            send_to_char( "You are too high for this group.\n\r", gch );
            xp = (xp*(low_level+21))/(2*high_level); 
        }
        
        if ( high_level - gch->level > 16 )
        {
            send_to_char( "You are too low for this group.\n\r", gch );
            xp = (xp*(low_level+17))/(2*high_level); 
        }
        
        if ( gch->alignment - low_align > 1200 )
        {
            send_to_char( "You are too moral for this group.\n\r", gch );
            xp/=2;
        }
        
        if ( high_align - gch->alignment > 1500 )
        {
            send_to_char( "You are too immoral for this group.\n\r", gch );
            xp/=2;
        }
	*/

        if (members>1)
        {
            xp = (xp*(4000-high_align+low_align))/4000;
            xp = (xp*(low_level+99))/(high_level+99);
        }

/*	if ( ch != gch && is_same_player(ch, gch) )
	{
	    sprintf( buf, "Multiplay: %s gaining experience from %s's kill",
		     gch->name, ch->name );
	    cheat_log( buf );
	    wiznet(buf, ch, NULL, WIZ_CHEAT, 0, LEVEL_IMMORTAL);
	}
	else*/
	    gain_exp( gch, xp );

        if (!IS_NPC(gch) && !IS_IMMORTAL(gch) &&
            (  gch->alignment < clan_table[gch->clan].min_align 
            || gch->alignment > clan_table[gch->clan].max_align))
        {
            send_to_char("Your alignment has made you unwelcome in your clan!\n\r", gch);
            sprintf(log_buf, "%s has become too %s for clan %s!",
                gch->name, 
                gch->alignment < clan_table[gch->clan].min_align ? "{rEvil{x" : "{wGood{x", 
                capitalize(clan_table[gch->clan].name));
            
            info_message(gch, log_buf, TRUE);

            gch->clan = 0;
            gch->pcdata->clan_rank = 0;

            check_clan_eq(gch);
        }
        
        for ( obj = gch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            if ( obj->wear_loc == WEAR_NONE )
                continue;
            
            if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(gch)    )
                ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(gch)    )
                ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(gch) ) )
            {
                act( "You are zapped by $p.", gch, obj, NULL, TO_CHAR );
                act( "$n is zapped by $p.",   gch, obj, NULL, TO_ROOM );
                obj_from_char( obj );
		/* move to inventory --Bobble
                obj_to_room( obj, gch->in_room );
		*/
                obj_to_char( obj, gch );
            }
            
        }

	/* moved to catch kills by NPC groupies  --Bobble
	if (!IS_NPC(gch) && IS_SET(gch->act, PLR_QUESTOR) && IS_NPC(victim))
	{
	    if (gch->pcdata->questmob == victim->pIndexData->vnum)
	    {
		send_to_char("You have almost completed your QUEST!\n\r",gch);
		send_to_char("Return to the questmaster before your time runs out!\n\r",gch);
		gch->pcdata->questmob = -1;
	    }
	}
	*/
    }
    return;
}

/* returns the 'effective strength' of a char */
int level_power( CHAR_DATA *ch )
{
    int pow;
    if ( IS_NPC(ch) )
	return ch->level;
    else
    {
	pow = ch->level * (50 + ch->pcdata->remorts) / 50;
	if ( ch->level >= 90 )
	    pow += 10;
	return pow;
    }
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer ( by gain_align percentage )
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int gain_align )
{
    int xp,base_exp;
    
    int valign, galign,level_range;
    int change;
    
    int max, min;
    float g, v;

    int bonus = 0;

    /* safety net */
    if ( IS_NPC(gch) || !IS_NPC(victim) || IS_SET(victim->act, ACT_NOEXP) )
	return 0;

    level_range = victim->level - level_power(gch);
    
    /* compute the base exp */
    /*
    switch (level_range)
    {
    default :   base_exp =   0;     break;
    case -15 :  base_exp =   1;     break;
    case -14 :  base_exp =   2;     break;
    case -13 :  base_exp =   3;     break;
    case -12 :  base_exp =   4;     break;
    case -11 :  base_exp =   5;     break;
    case -10 :  base_exp =   6;     break;
    case -9 :   base_exp =   7;     break;
    case -8 :   base_exp =   9;     break;
    case -7 :   base_exp =  11;     break;
    case -6 :   base_exp =  17;     break;
    case -5 :   base_exp =  26;     break;
    case -4 :   base_exp =  35;     break;
    case -3 :   base_exp =  44;     break;
    case -2 :   base_exp =  53;     break;
    case -1 :   base_exp =  62;     break;
    case  0 :   base_exp =  70;     break;
    case  1 :   base_exp =  78;     break;
    case  2 :   base_exp =  86;     break;
    case  3 :   base_exp =  93;     break;
    case  4 :   base_exp = 100;     break;
    } 
    
    if (level_range > 4)
        base_exp = 97 + 12 * (level_range - 4);
    */

    /* Don't force players to kill super-strong mobs --Bobble */
    base_exp = 100 + 10 * level_range;
    base_exp = UMAX(0, base_exp);

    /* shoot down super-levelling mobs */
    if ( IS_SET(victim->vuln_flags, VULN_WEAPON) )
	base_exp -= base_exp/20;
    if ( IS_SET(victim->vuln_flags, VULN_MAGIC) )
	base_exp -= base_exp/20;

    /* reward for tough mobs */
    if ( IS_SET(victim->res_flags, RES_WEAPON)
	 || IS_SET(victim->imm_flags, IMM_WEAPON) )
	base_exp += base_exp/20;
    if ( IS_SET(victim->res_flags, RES_MAGIC)
	 || IS_SET(victim->imm_flags, IMM_MAGIC) )
	base_exp += base_exp/20;
    if ( IS_SET(victim->pIndexData->affect_field, AFF_SANCTUARY) )
	base_exp += base_exp/3;
    if ( IS_SET(victim->pIndexData->affect_field, AFF_HASTE) )
	base_exp += base_exp/10;
    if ( IS_SET(victim->off_flags, OFF_FADE) )
	base_exp += base_exp/4;
    else if ( IS_SET(victim->pIndexData->affect_field, AFF_FADE)
	      || IS_SET(victim->pIndexData->affect_field, AFF_CHAOS_FADE) )
	base_exp += base_exp/5;
    if ( IS_SET(victim->off_flags, OFF_DODGE) )
	base_exp += base_exp/10;
    if ( IS_SET(victim->off_flags, OFF_PARRY) )
	base_exp += base_exp/10;
    if ( IS_SET(victim->off_flags, OFF_FAST) )
	base_exp += base_exp/10;
    if ( IS_SET(victim->off_flags, OFF_BASH) )
	base_exp += base_exp/20;
    if ( IS_SET(victim->off_flags, OFF_TAIL) )
	base_exp += base_exp/20;
    if ( IS_SET(victim->pIndexData->act, ACT_AGGRESSIVE) )
	base_exp += base_exp/10;

    /* do alignment computations */
    galign = gch->alignment;
    valign = victim->alignment;
    
    if ( IS_SET(victim->act, ACT_NOALIGN) || gain_align == 0
	 || victim->pIndexData->vnum == gch->pcdata->questmob )
	change = 0;
    else if (galign>300)
    {
        if (valign>-100) change= ((-100 - valign)*galign)/1600;
        else if (galign<-valign)
        {
            change = (-valign -galign);
            change = UMIN(change, (change*base_exp)/500);
        }
        else change = 0;
    }
    else if (galign<300)
    {
        if (valign<-200) change=((-200 - valign)*base_exp)/(800);
        else if (galign>(-500-valign/2))
        {
            change = (-500-valign/2 -galign);
            change = UMAX(change, (change*base_exp)/(300));
        }
        else change = 0;
    }
    else
    {
        min = -300 -galign/3;
        max = 200 - 2*galign/3;
        if (valign>max)
        {
            change = (max - valign);
            change = UMAX(change, (change*base_exp)/(400));
        }
        else if (valign<min)
        {
            change = (min - valign);  
            change = UMAX(change, (change*base_exp)/(600));
        }
        else change = 0;
    }
    
    /* general reduction of align change --Bobble */
    change /= 10;

    if ( gain_align > 0 )
	change_align(gch, change * gain_align/100);
    
    g = (int)(((float)galign)/1000.0);
    v = (int)(((float)valign)/1000.0);
    
    if (IS_SET(victim->act,ACT_NOALIGN))
        xp = base_exp;
    /* seems very strange --Bobble
    else if (galign > 0)
    {
        if (valign >0)  xp=(int)(base_exp * (.9 +.2*v -.1*g -.5*g*v));
        else            xp=(int)(base_exp * (.9 -.2*v -.1*g -.4*g*v));    
    }
    else
    {
        if (valign>0)   xp=(int)(base_exp * (.9 +.2*v -.1*g));   
        else            xp=(int)(base_exp * (.9 -.2*v -.1*g -.6*g*v));    
    }
    */
    else if ( g > 0 )
	xp = (int) (base_exp * (1 - .2*g*v));
    else
	xp = (int) (base_exp * (1 - .1*g*v));

    /* reduction of xp */
    xp = xp * 3/4;

    /* bonus for newbies */
    if ( gch->pcdata->remorts == 0 )
	xp += xp * (100 - gch->level) / 100;
    /* more exp at the low levels */
    if (gch->level < 16)
        xp = 32 * xp / (gch->level + 16);
    /* less at high */
    if (gch->level > 60 )
        xp =  40 * xp / (gch->level - 20 );
    if (gch->level > 90 )
        xp= (4*xp)/(gch->level-87);

/* The above number, 4 x XP was changed from a 3. This gives more XP at hero level 
   Astark 6-1-12 */
    
    /* reduce for playing time 
       {
       time_per_level = 4 *
       (gch->played + (int) (current_time - gch->logon))/3600/ gch->level;
       time_per_level = URANGE(2,time_per_level,12);
       if (gch->level < 15)  
       time_per_level = UMAX(time_per_level,(15 - gch->level));
       xp = xp * time_per_level / 12;
       } */
    
    /* randomize the rewards */
    /*xp = number_range (xp * 3/4, xp * 5/4);*/
    
    /* adjust for grouping */
    /*xp = xp * gch->level/( UMAX(1,(2*gch->level + total_levels)/3) );*/

    /* normal pkillers get 10% exp bonus, hardcore pkillers 20% */
    if ( IS_SET(gch->act, PLR_PERM_PKILL) )
	if ( IS_SET(gch->act, PLR_HARDCORE) )
	    bonus += 20;
	else
	    bonus += 10;

    /* roleplayers get 10% exp bonus, since they are open to being killed by players */
    if ( IS_SET(gch->act, PLR_RP) )
	bonus += 10;

    /* religion bonus */
    bonus += get_religion_bonus(gch);
    
    /* bonus for AFF_LEARN */
    if ( IS_AFFECTED(gch, AFF_LEARN) )
	bonus += 50;

    if ( IS_AFFECTED(gch, AFF_HALLOW) )
        bonus += 20;

    xp += xp * bonus / 100;

    return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf[256], buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
	char *victmeter, *chmeter;
    char punct;
    long gag_type = 0;
    int sn;
    
    if (ch == NULL || victim == NULL)
        return;

	sprintf(buf, " for %d damage", dam);
	chmeter = ((IS_AFFECTED(ch, AFF_BATTLE_METER) && (dam>0)) ? buf : "");
	victmeter = ((IS_AFFECTED(victim, AFF_BATTLE_METER) && (dam>0)) ? buf : "");

	if (dam<100)
	if (dam < 39)
	{
	    if ( dam < 1 )
	    { 
		vs = "miss"; vp = "misses";
		if ( is_normal_hit(dt) )
		    gag_type = GAG_MISS;
	    }
        else if ( dam <   2 ) { vs = "{mbother{ ";  vp = "{mbothers{ "; }
        else if ( dam <   5 ) { vs = "{mscratch{ "; vp = "{mscratches{ ";   }
        else if ( dam <   8 ) { vs = "{mbruise{ ";  vp = "{mbruises{ "; }
        else if ( dam <  11 ) { vs = "{bglance{ ";  vp = "{bglances{ "; }
        else if ( dam <  15 ) { vs = "{bhurt{ ";    vp = "{bhurts{ ";   }
        else if ( dam <  20 ) { vs = "{Bgraze{ ";   vp = "{Bgrazes{ ";  }
        else if ( dam <  27 ) { vs = "{Bhit{ ";     vp = "{Bhits{ ";    }
        else if ( dam <  31 ) { vs = "{Binjure{ ";  vp = "{Binjures{ "; }
        else if ( dam <  35 ) { vs = "{Cwound{ ";   vp = "{Cwounds{ ";  }
        else { vs = "{CPUMMEL{ ";  vp = "{CPUMMELS{ "; }
		} else {
        if ( dam <  44 ) { vs = "{CMAUL{ ";        vp = "{CMAULS{ ";   }
        else if ( dam <  48 ) { vs = "{GDECIMATE{ ";    vp = "{GDECIMATES{ ";   }
        else if ( dam <  52 ) { vs = "{GDEVASTATE{ ";   vp = "{GDEVASTATES{ ";}
        else if ( dam <  56 ) { vs = "{GMAIM{ ";        vp = "{GMAIMS{ ";   }
        else if ( dam <  60 ) { vs = "{yMANGLE{ ";  vp = "{yMANGLES{ "; }
        else if ( dam <  64 ) { vs = "{yDEMOLISH{ ";    vp = "{yDEMOLISHES{ ";}
        else if ( dam <  70 ) { vs = "*** {yMUTILATE{  ***"; vp = "*** {yMUTILATES{  ***";}
        else if ( dam <  80 ) { vs = "*** {YPULVERIZE{  ***";   vp = "*** {YPULVERIZES{  ***";}
        else if ( dam <  90 ) { vs = "=== {YDISMEMBER{  ==="; vp = "=== {YDISMEMBERS{  ===";}
        else { vs = "=== {YDISEMBOWEL{  ==="; vp = "=== {YDISEMBOWELS{  ===";}
		}
    else
		if (dam < 220)
		{
        if ( dam <  110) { vs = ">>> {rMASSACRE{  <<<"; vp = ">>> {rMASSACRES{  <<<";}
        else if ( dam < 120)  { vs = ">>> {rOBLITERATE{  <<<"; vp = ">>> {rOBLITERATES{  <<<";}
        else if ( dam < 135)  { vs = "{r<<< ANNIHILATE >>>{ "; vp = "{r<<< ANNIHILATES >>>{ ";}
        else if ( dam < 150)  { vs = "{r<<< DESTROY >>>{ "; vp = "{r<<< DESTROYS >>>{ ";}
        else if ( dam < 165)  { vs = "{R!!! ERADICATE !!!{ "; vp = "{R!!! ERADICATES !!!{ ";}
        else if ( dam < 190)  { vs = "{R!!! LIQUIDATE !!!{ "; vp = "{R!!! LIQUIDATES !!!{ ";}
        else { vs = "{RXXX VAPORIZE XXX{ "; vp = "{RXXX VAPORIZES XXX{ ";}
		} else {
        if ( dam < 250)  { vs = "{RXXX DISINTEGRATE XXX{ "; vp = "{RXXX DISINTEGRATES XXX{ ";}
        else if ( dam < 300)  { vs = "do {+SICKENING{  damage to"; vp = "does {+SICKENING{  damage to";}
        else if ( dam < 400)  { vs = "do {+INSANE{  damage to"; vp = "does {+INSANE{  damage to";}
        else if ( dam < 600)  { vs = "do {+UNSPEAKABLE{  things to"; vp = "does {+UNSPEAKABLE{  things to";}
        else if ( dam < 1000)  { vs = "do {+{%BLASPHEMOUS{  things to"; vp = "does {+{%BLASPHEMOUS{  things to";}
        else if ( dam < 1500)  { vs = "do {+{%UNBELIEVABLE{  things to"; vp = "does {+{%UNBELIEVABLE{  things to";}
        else    { vs = "do {+{%INCONCEIVABLE{  things to"; vp = "does {+{%INCONCEIVABLE{  things to";}
		}
    
    punct   = (dam < 31) ? '.' : '!';
    
    if ( dt == TYPE_HIT )
    {
        if (ch  == victim)
        {
            sprintf( buf1, "$n %s $melf%c",vp,punct);
            sprintf( buf2, "You %s yourself%s%c",vs,chmeter,punct);
        }
        else
        {
            sprintf( buf1, "$n %s $N%c",  vp, punct );
            sprintf( buf2, "You %s $N%s%c", vs, chmeter, punct );
            sprintf( buf3, "$n %s you%s%c", vp, victmeter, punct );
        }
    }
    else
    {
        if ( dt >= 0 && dt < MAX_SKILL )
            attack  = skill_table[dt].noun_damage;
        
        else if (ch->stance!=0&&((stances[ch->stance].martial && get_eq_char( ch, WEAR_WIELD ) == NULL )
            || (ch->stance == STANCE_KORINNS_INSPIRATION || ch->stance == STANCE_PARADEMIAS_BILE)))
            attack=stances[ch->stance].verb;
        
        else if ( dt >= TYPE_HIT
            && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
            attack  = attack_table[dt - TYPE_HIT].noun;
        else
        {
            bug( "Dam_message: bad dt %d.", dt );
            dt  = TYPE_HIT;
            attack  = attack_table[0].name;
        }
        
        if (immune)
        {
            if (ch == victim)
            {
                sprintf(buf1,"$n is unaffected by $s own %s.",attack);
                sprintf(buf2,"Luckily, you are immune to that.");
            } 
            else
            {
                sprintf(buf1,"$N is unaffected by $n's %s!",attack);
                sprintf(buf2,"$N is unaffected by your %s!",attack);
                sprintf(buf3,"$n's %s is powerless against you.",attack);
            }
        }
        else
        {
            if (ch == victim)
            {
                sprintf( buf1, "$n's %s %s $m%c",attack,vp,punct);
                sprintf( buf2, "Your %s %s you%s%c", attack, vp, chmeter, punct);
            }
            else
            {
                sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct );
                sprintf( buf2, "Your %s %s $N%s%c",  attack, vp, chmeter, punct );
                sprintf( buf3, "$n's %s %s you%s%c", attack, vp, victmeter, punct );
            }
        }
    }
    
    if ( immune )
	gag_type = GAG_IMMUNE;

    if ( dt < MAX_SKILL ) /* Make sure in bounds of array before we check! */
      if( skill_table[dt].pgsn != NULL )
    
      {
	  sn = *skill_table[dt].pgsn;
          if( sn == gsn_electrocution
	  || sn == gsn_immolation
	  || sn == gsn_absolute_zero
	  || sn == gsn_epidemic
	  || sn == gsn_quirkys_insanity )
	  gag_type = GAG_AURA;
      }

    if (ch == victim)
    {
        act_gag(buf1,ch,NULL,NULL,TO_ROOM, gag_type);
        act_gag(buf2,ch,NULL,NULL,TO_CHAR, gag_type);
    }
    else
    {
        act_gag( buf1, ch, NULL, victim, TO_NOTVICT, gag_type);
        act_gag( buf2, ch, NULL, victim, TO_CHAR, gag_type);
        act_gag( buf3, ch, NULL, victim, TO_VICT, gag_type);
    }
    
    return;
}

/* TRUE if ch is involved in a pkill battle */
bool in_pkill_battle( CHAR_DATA *ch )
{
    CHAR_DATA *opp;
    
    if ( ch->in_room == NULL )
	return FALSE;

    if (ch->fighting != NULL && !IS_NPC(ch->fighting))
        return TRUE;
    
    for (opp = ch->in_room->people; opp != NULL; opp = opp->next_in_room)
        if (opp->fighting == ch && !IS_NPC(opp))
            return TRUE;
        
    return FALSE;
}



extern char *const dir_name[];

bool check_lasso( CHAR_DATA *victim );
void check_back_leap( CHAR_DATA *victim );

void do_flee( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[80];
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    int choice, dir, chance, skill, num;
    CHAR_DATA *opp;
    int opp_value, max_opp_value = 0;
    //static bool no_flee = FALSE;
    bool ch_in_pkill_battle = in_pkill_battle( ch );
    
    if ( IS_AFFECTED(ch, AFF_FLEE) )
       return;

    if ( (victim = ch->fighting) == NULL )
    {
        if (ch->position == POS_FIGHTING)
	    set_pos( ch, POS_STANDING );
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }
    
    if (IS_AFFECTED(ch, AFF_ENTANGLE) && (number_percent() > (get_curr_stat(ch, STAT_LUC)/10)))
    {
	send_to_char( "The plants entangling you hold you in place!\n\r", ch );
	return;
    }

    if (ch->position < POS_FIGHTING)
    {
	send_to_char( "You can't flee when you're not even standing up!\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( (was_in = ch->in_room) == NULL )
	return;
    
    if (arg[0] && (number_percent()<=get_skill(ch, gsn_retreat)))
    {
	for (dir=0; dir<MAX_DIR; dir++)
	    if (!str_prefix(arg, dir_name[dir]))
		break;

	if (dir >= MAX_DIR)
	{
	    send_to_char("That isnt a direction!\n\r", ch);
	    return;
	}

	check_improve(ch, gsn_retreat, TRUE, 4);
	chance = 5;
    }
    else
    {
	for (dir=0, num=0; dir<MAX_DIR; dir++)
	    if (!(( pexit = was_in->exit[dir] ) == 0
		  || pexit->u1.to_room == NULL
		  || (IS_SET(pexit->exit_info, EX_CLOSED)
		      && (IS_SET(pexit->exit_info, EX_NOPASS)
			  || !IS_AFFECTED(ch, AFF_PASS_DOOR)))
		  || (IS_NPC(ch) && IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
        /* Check added so that mobs can't flee into a safe room. Causes problems
           with resets, quests, and leveling - Astark Dec 2012 */
                  || (IS_NPC(ch) && IS_SET(pexit->u1.to_room->room_flags, ROOM_SAFE)))))
	    {
		if ( number_range(0, num) == 0 )
		    choice = dir;
		num++;
	    }
	
	if (num==0)
	{
	    send_to_char("There is nowhere to run!\n\r", ch);
	    return;
	}
	
	chance = 5*num;
	dir = choice;
	/*
	num = number_range(1, num);
	for (dir=0; (dir<MAX_DIR) && (num>0); dir++)
	    if (!(( pexit = was_in->exit[dir] ) == 0
		  || pexit->u1.to_room == NULL
		  || IS_SET(pexit->exit_info, EX_CLOSED)
		  || (IS_NPC(ch) && IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB))))
		num--;
	dir--;
	*/
    }

    chance += 25+get_skill(ch, gsn_flee)/2+get_skill(ch, gsn_retreat)/4;

    /* get stats from strongest opponent */
    for (opp = ch->in_room->people; opp != NULL; opp = opp->next_in_room)
    {
	if (opp->fighting != ch)
	    continue;
	
	opp_value = 2 * opp->level + 4 * get_curr_stat(opp, STAT_AGI);
	
	if ( opp_value > max_opp_value )
	    max_opp_value = opp_value;
    }

    chance *= 1000 - max_opp_value + 2 * ch->level + 
	3 * get_curr_stat(ch, STAT_AGI) + get_curr_stat(ch, STAT_LUC);
    
    chance /= 2; /* only 50% base chance */

    /* consider entrapment & ambush stance for all opponents fighting ch */
    for (opp = ch->in_room->people; opp != NULL; opp = opp->next_in_room)
    {
	if (opp->fighting != ch)
	    continue;
	
	if (skill=get_skill(opp, gsn_entrapment))
	{
	    chance = 100*chance/(100+skill);
	    check_improve(opp, gsn_entrapment, TRUE, 8);
	}
	
	if (opp->stance == STANCE_AMBUSH)
	    chance /= 2;
    }

    if (ch->daze > 0)
	chance /= 2;
    
    if (ch->slow_move > 0)
	chance /= 2;
    
    if (IS_NPC(ch) || (ch->stance == STANCE_BUNNY))
	chance += 10000;
    
    if (chance > 40000)
	chance = (chance+40000)/2;
    
    if (max_opp_value > 0 && number_percent()*1000 > chance)
    {
	WAIT_STATE(ch, 6);
	send_to_char( "PANIC! You couldn't escape!\n\r", ch );
	check_improve(ch, gsn_flee, FALSE, 4);
	return;
    }
    
    if (is_affected(ch, gsn_net))
    {
	/* Chance of breaking the net:  str/15 < 13.5% */
	if( number_percent() < (get_curr_stat(ch, STAT_STR)/15) )
	{
	    send_to_char( "You rip the net apart!\n\r", ch );
	    act("$n rips the net apart!", ch, NULL, NULL, TO_ROOM);
	    affect_strip( ch, gsn_net );
	}
	/* Chance of struggling out of the net:  dex/15 < 13.5% */
	else if( number_percent() < (get_curr_stat(ch, STAT_DEX)/15) )
	{
	    send_to_char( "You struggle free from the net!\n\r", ch );
	    act("$n struggles free from the net!", ch, NULL, NULL, TO_ROOM);
	    affect_strip( ch, gsn_net );
	}
	/* Chance of fleeing while still trapped in the net:  (agi+luc)/30 < 13.5% */
	/* (This last check does not allow flee to occur.) */
	else if( number_percent() > (get_curr_stat(ch, STAT_AGI)+get_curr_stat(ch, STAT_LUC))/30 )
	{
	    send_to_char( "You struggle in the net, and can't seem to get away.\n\r", ch );
	    act("$n struggles in the net.", ch, NULL, NULL, TO_ROOM);
	    return;
	}
    }

    /* opponents may catch them and prevent fleeing */
    if ( check_lasso(ch) )
	return;
    /* prevent wimpy-triggered recursive fleeing */
    SET_AFFECT(ch, AFF_FLEE);
    /* opponents may leap on fleeing player and kill him */
    check_back_leap(ch);
    REMOVE_AFFECT(ch, AFF_FLEE);

    if (ch->fighting == NULL)
	return;
    
    dir = move_char(ch, dir, FALSE);    
    now_in=ch->in_room;
    
    if (now_in==was_in)
    {
	WAIT_STATE(ch, 6);
	send_to_char( "You get turned around and flee back into the room!\n\r", ch );
	return;
    }

    /* char might have been transed by an mprog */
    if ( dir == -1 )
	sprintf(buf, "$n has fled!");
    else
	sprintf(buf, "$n has fled %s!", dir_name[dir]);
    check_improve(ch, gsn_flee, TRUE, 4);
        
    ch->in_room = was_in;
    act(buf, ch, NULL, NULL, TO_ROOM);
    ch->in_room = now_in;
        
    if ( dir == -1 )
	sprintf(buf, "You flee from combat!\n\r", dir_name[dir]);
    else
	sprintf(buf, "You flee %s from combat!\n\r", dir_name[dir]);
    //send_to_char(buf, ch);
    act( buf, ch, NULL, NULL, TO_CHAR );

    if ( !IS_NPC(ch) )
    {
        if( (ch->class == 1) && (number_percent() < ch->level ) )
            send_to_char( "You snuck away safely.\n\r", ch);
        else if ( !IS_HERO(ch) && !IS_SET(ch->act, PLR_WAR) )
        {
            send_to_char( "You lost 10 exp.\n\r", ch); 
            gain_exp( ch, -10 );
        }
    }

    if (!IS_NPC(ch) && ch_in_pkill_battle && ch->pcdata != NULL)
        ch->pcdata->pkill_timer = UMAX(ch->pcdata->pkill_timer, 5 * PULSE_VIOLENCE);

    /*
    if (now_in!=was_in)
        stop_fighting( ch, TRUE );
    */
}

/* opponents can throw a lasso at fleeing player and prevent his fleeing */
bool check_lasso( CHAR_DATA *victim )
{
    CHAR_DATA *opp, *next_opp;
    OBJ_DATA *lasso;
    AFFECT_DATA af;
    int skill, chance;
    
    if (victim == NULL || victim->in_room == NULL)
    {
	bug("check_lasso: NULL victim or NULL in_room", 0);
	return FALSE;
    }
    
    for (opp = victim->in_room->people; opp != NULL; opp = next_opp)
    {
	next_opp = opp->next_in_room;

	if (opp->fighting != victim)
	    continue;
	
	if ( (lasso=get_eq_char(opp, WEAR_HOLD)) == NULL
	     || lasso->item_type != ITEM_HOGTIE )
	    continue;

	skill = get_skill(opp, gsn_hogtie);
	if (number_percent() > skill)
	    continue;

	act( "$n throws a lasso at you!", opp, NULL, victim, TO_VICT    );
	act( "You throw a lasso at $N!", opp, NULL, victim, TO_CHAR    );
	act( "$n throws a lasso at $N!", opp, NULL, victim, TO_NOTVICT );

	chance = get_skill(victim, gsn_avoidance) - skill / 2;
	if (number_percent() < chance)
	{
	    act( "You avoid $n!",  opp, NULL, victim, TO_VICT    );
	    act( "$N avoids you!", opp, NULL, victim, TO_CHAR    );
	    act( "$N avoids $n!",  opp, NULL, victim, TO_NOTVICT );
	    check_improve(victim,gsn_avoidance,TRUE,1);
	    continue;
	}

	chance = skill/2 + (get_curr_stat(opp, STAT_DEX) -
			    get_curr_stat(victim,STAT_AGI))/8;
	if ( number_percent() <= chance )
	{
	    act( "$n catches you!", opp, NULL, victim, TO_VICT    );
	    act( "You catch $N!", opp, NULL, victim, TO_CHAR    );
	    act( "$n catches $N!", opp, NULL, victim, TO_NOTVICT );

	    check_improve(opp,gsn_hogtie,TRUE,2);

	    victim->stance = 0;
	    if ( !is_affected(victim, gsn_hogtie) )
	    {
		af.where    = TO_AFFECTS;
		af.type     = gsn_hogtie;
		af.level    = opp->level;
		af.duration = 0;
		af.location = APPLY_AGI;
		af.modifier = -20;
		af.bitvector = AFF_SLOW;
		affect_to_char(victim,&af);
	    }
	    WAIT_STATE( victim, 6 );
	    return TRUE;
	}
    }   
    
    return FALSE;
}

/* opponents can leap on the victim and kill it */
void check_back_leap( CHAR_DATA *victim )
{
    CHAR_DATA *opp, *next_opp;
    OBJ_DATA *wield;
    int chance;
    
    if (victim == NULL || victim->in_room == NULL)
    {
	bug("check_back_leap: NULL victim or NULL in_room", 0);
	return;
    }
    
    for (opp = victim->in_room->people; opp != NULL; opp = next_opp)
    {
	next_opp = opp->next_in_room;

	if ( opp->fighting != victim || !can_see(opp, victim) )
	    continue;
	
	wield = get_eq_char( opp, WEAR_WIELD );
	/* ranged weapons get off one shot */
	if ( wield != NULL
	     && (wield->value[0] == WEAPON_GUN || wield->value[0] == WEAPON_BOW) )
	{
	    act( "$n shoots at your back!", opp, NULL, victim, TO_VICT    );
	    act( "You shoot at $N's back!", opp, NULL, victim, TO_CHAR    );
	    act( "$n shoots at $N's back!", opp, NULL, victim, TO_NOTVICT );
	    one_hit( opp, victim, TYPE_UNDEFINED, FALSE );
	    CHECK_RETURN( opp, victim );
	    if ( (wield = get_eq_char(opp, WEAR_SECONDARY)) != NULL
		 && wield->value[0] == WEAPON_GUN && number_bits(1) )
		one_hit( opp, victim, TYPE_UNDEFINED, TRUE );
	    continue;
	}

	chance = get_skill(opp, gsn_back_leap);
	if ( opp->stance != STANCE_AMBUSH )
	    chance /= 2;

	if (number_percent() >= chance)
	    continue;

	act( "$n leaps at your back!", opp, NULL, victim, TO_VICT    );
	act( "You leap at $N's back!", opp, NULL, victim, TO_CHAR    );
	act( "$n leaps at $N's back!", opp, NULL, victim, TO_NOTVICT );
	check_improve(opp,gsn_back_leap,TRUE,1);    

	chance = get_skill(victim, gsn_avoidance) - chance / 2;
	if (number_percent() < chance)
	{
	    act( "You avoid $n!",  opp, NULL, victim, TO_VICT    );
	    act( "$N avoids you!", opp, NULL, victim, TO_CHAR    );
	    act( "$N avoids $n!",  opp, NULL, victim, TO_NOTVICT );
	    check_improve(victim,gsn_avoidance,TRUE,1);
	    continue;
	}

	/* behead checking */
	if ( wield != NULL && wield->value[0] == WEAPON_DAGGER
	     && !number_bits(4)
	     && (opp->stance == STANCE_AMBUSH || number_bits(1)))
	{
	    int chance = get_skill(opp, gsn_assassination);
	    if (number_percent() < chance)
	    {
		act("$n drives $s weapon deep into your neck till it snaps!",
		    opp,NULL,victim,TO_VICT);
		act("You drive your weapon deep into $N's neck till it snaps!",
		    opp,NULL,victim,TO_CHAR);
		act("$n drives $s weapon deep into $N's neck till it snaps!",
		    opp,NULL,victim,TO_NOTVICT);
		behead(opp,victim);
		check_improve(opp,gsn_assassination,TRUE,1);
		return; 
	    }
	}
    
	/* now the attacks */
	one_hit(opp, victim, gsn_back_leap, FALSE);
	if (opp->fighting == victim && get_eq_char(opp, WEAR_SECONDARY))
	    one_hit(opp, victim, gsn_back_leap, TRUE);
    }
}

/* checks if victim has a bodyguard that leaps in
 * if so, returns the bodyguard, else the victim
 * used by do_kill, do_murder and aggr_update
 */
CHAR_DATA* check_bodyguard( CHAR_DATA *attacker, CHAR_DATA *victim )
{
  CHAR_DATA *ch;
  int chance;
  int ass_skill = get_skill(attacker, gsn_assassination);
   
  for (ch = victim->in_room->people; ch != NULL; ch = ch->next_in_room)
  {
      if ( !wants_to_rescue(ch)
	   || (ch->leader != NULL && ch->leader != victim)
	   || !is_same_group(ch, victim)
	   || ch == victim || ch == attacker )
	  continue;
      if (is_safe_spell(attacker, ch, FALSE)
	  || ch->position <= POS_SLEEPING 
	  || !check_see(ch, attacker))
	  continue;

      chance = 25 + get_skill(ch, gsn_bodyguard) / 2 - ass_skill / 4;
      chance += ch->level - attacker->level;
      if (number_percent() < chance)
      {
	  act( "You jump in, trying to protect $N.", ch, NULL, victim, TO_CHAR );
	  act( "$n jumps in, trying to protect you.", ch, NULL, victim, TO_VICT );
	  act( "$n jumps in, trying to protect $N.", ch, NULL, victim, TO_NOTVICT );
	  check_improve(ch, gsn_bodyguard, TRUE, 1);
	  check_killer(ch, attacker);
	  return ch;
      }
      else
	  check_improve(attacker, gsn_assassination, TRUE, 1);
  }
  return victim;
}

void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    bool was_fighting;
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        send_to_char( "Kill whom?\n\r", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER)
	     && !IS_SET(victim->act, PLR_THIEF) )
        {
            send_to_char( "You must MURDER a player.\n\r", ch );
            return;
        }
    }
    if ( victim == ch )
    {
        send_to_char( "You hit yourself.  Ouch!\n\r", ch );
        /*multi_hit( ch, ch, TYPE_UNDEFINED );*/
        return;
    }

    if ( is_safe( ch, victim ) )
        return;
    
/* These checks occur in is_safe:
    if ( check_kill_steal(ch,victim) )
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
    
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
        act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
        return;
    }
*/
    
    if ( ch->fighting == victim )
    {
        send_to_char( "You do the best you can!\n\r", ch );
        return;
    }
    
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_killer( ch, victim );
    victim = check_bodyguard( ch, victim );

    was_fighting = ch->fighting != NULL;
    set_fighting( ch, victim );
    
    if ( was_fighting ) 
        return;
    
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

void do_die( CHAR_DATA *ch, char *argument )
{
    if( ch->hit > 0 || NOT_AUTHED(ch) )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if( ch->pcdata != NULL && ch->pcdata->pkill_timer > 0 )
    {
	send_to_char( "Relax... your imminent death is in the hands of your killer.\n\r", ch );
	return;
    }

    if( IS_SET(ch->act,PLR_WAR) )
    {
	send_to_char( "The only way to leave warfare is to be killed by someone else.\n\r", ch );
	return;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);
    send_to_char( "You let your life energy slip out of your body.\n\r", ch );
    send_to_char( "You have been KILLED!!\n\r\n\r", ch );
    handle_death( ch, ch );
    return;
}

void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    bool was_fighting;
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        send_to_char( "Murder whom?\n\r", ch );
        return;
    }
    
    /*
    if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
        return;
    */    

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    
    if ( victim == ch )
    {
        send_to_char( "Suicide is a mortal sin.\n\r", ch );
        return;
    }
    
    if ( is_safe( ch, victim ) )
        return;

/* These checks occur in is_safe:
    if ( check_kill_steal(ch,victim) )
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
    
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
        act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
        return;
    }
*/
    
    if ( ch->fighting==victim)
    {
        send_to_char( "You do the best you can!\n\r", ch );
        return;
    }
    
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    /*
    if (IS_NPC(ch))
        sprintf(buf, "Help! I am being attacked by %s!", ch->short_descr);
    else
        sprintf( buf, "Help!  I am being attacked by %s!", ch->name );
    */
    sprintf( buf, "Help!  I am being attacked by %s!", PERS(ch, victim) );
    
    do_yell( victim, buf );
    check_killer( ch, victim );
    victim = check_bodyguard( ch, victim );

    was_fighting = ch->fighting != NULL;
    set_fighting( ch, victim );
    
    if ( was_fighting ) 
        return;
    
    if ((get_skill(victim, gsn_quick_draw)) != 0 && IS_AWAKE(victim))
    {
        chance = (2*get_skill(victim, gsn_quick_draw)/3);
        chance += (get_curr_stat(victim, STAT_DEX) - get_curr_stat(ch, STAT_AGI)) / 6;
	if (get_eq_char(ch, WEAR_WIELD)==NULL)
	    chance = 0;
        else if (get_weapon_sn(victim) != gsn_gun)
	    chance /= 2;
        if (number_percent() < chance)
        {
                act("You get the quick draw on $n!", ch, NULL, victim, TO_VICT);
                act("$N gets the quick draw on you!", ch, NULL, victim, TO_CHAR);
                act("$N gets the quick draw on $n!", ch, NULL, victim, TO_NOTVICT);  
                check_improve(victim, gsn_quick_draw, TRUE, 1);
		check_killer(ch, victim);
                multi_hit(victim, ch, TYPE_UNDEFINED);
		if (victim->fighting==ch)
		    multi_hit(victim, ch, TYPE_UNDEFINED);
		return;
        } 
    } 

    if ((chance= get_skill(victim, gsn_avoidance))!=0 && IS_AWAKE(victim))
    {
        chance += get_curr_stat(victim,STAT_AGI)/8;
        chance -= get_curr_stat(ch,STAT_AGI)/8;
        chance = chance * 2/3;

        if (number_percent() <chance)
        {
            act( "You avoid $n!",  ch, NULL, victim, TO_VICT    );
            act( "$N avoids you!", ch, NULL, victim, TO_CHAR    );
            act( "$N avoids $n!",  ch, NULL, victim, TO_NOTVICT );
            check_improve(ch,gsn_avoidance,TRUE,1);
	    return;
        }
    } 

    multi_hit (ch, victim, TYPE_UNDEFINED); 
    return;    
}

int stance_cost( CHAR_DATA *ch, int stance )
{
    int cost = stances[stance].cost;
    int skill = get_skill(ch, *(stances[stance].gsn));

    return cost * (140-skill)/40;
}

void check_stance(CHAR_DATA *ch)
{
    int cost, skill;

    if (ch->stance == 0) return;
    
    if ( IS_NPC(ch) && ch->stance == ch->pIndexData->stance )
	return;
    /*
      cost = 0;
    else
      cost = stances[ch->stance].cost;

    if ( (skill = get_skill(ch, *(stances[ch->stance].gsn))) > 0 )
        cost += (cost*(100-skill))/40;
    */
    cost = stance_cost( ch, ch->stance );
    
    if (cost > ch->move)
    {
        send_to_char("You are too exhausted to keep up your fighting stance.\n\r", ch);
        ch->stance = 0;
        return;
    }
    
    if ( get_eq_char(ch, WEAR_WIELD) == NULL )
    {
        if ( !stances[ch->stance].martial )
        {
            send_to_char("You need a weapon for that stance.\n\r", ch);
            ch->stance = 0;
            return;
        }
    }
    else
    {
        if ( !stances[ch->stance].weapon )
        {
            send_to_char("You cant do that stance with a weapon.\n\r", ch);
            ch->stance = 0;
            return;
        }
    }

    if ( is_affected(ch, gsn_paroxysm) )
    {
        send_to_char("You're too hurt to maintain a stance.\n\r", ch);
        ch->stance = 0;
        return;
    }
    
    check_improve(ch,*(stances[ch->stance].gsn),TRUE,3);
    
    ch->move -= cost;

    /*Added by Korinn 1-19-99 */
    if (ch->stance == STANCE_FIREWITCHS_SEANCE)
    {
	int incr;
	/*   hp and mana will always be increased by at least 10.    *
	 * With greater skill, the increase could be as much as 25.  *
         * Recall: the cost in moves at 100% is 10mv (as of Sept/02) */
	incr   = UMAX( 10, 25 - (100-get_skill(ch,gsn_firewitchs_seance))/2 );
	ch->hit   = UMIN( ch->hit + incr, ch->max_hit );
	ch->mana  = UMIN( ch->mana + incr, ch->max_mana );
    }
}


void do_stance (CHAR_DATA *ch, char *argument)
{
    int i;
    bool is_pet;
    
    if (argument[0] == '\0')
    {
        if (ch->stance == 0)
            send_to_char("Which stance do you want to take?\n\r", ch);
        else
            send_to_char("You revert to your normal posture.\n\r", ch);

        ch->stance = 0;
        return;
    }
    
    for (i = 0; stances[i].name != NULL; i++)
        if (!str_prefix(argument, stances[i].name))
	    break;

    if ( IS_NPC(ch) && (IS_SET(ch->act, ACT_PET) || IS_AFFECTED(ch, AFF_CHARM)) )
	is_pet = TRUE;
    else
	is_pet = FALSE;

    if ( stances[i].name == NULL
	 || (!IS_NPC(ch) && get_skill(ch, *(stances[i].gsn))==0)
	 || (is_pet && i != ch->pIndexData->stance) )
    {
	ch->stance = 0;
	send_to_char("You do not know that stance.\n\r", ch);
	return;
    }
        
    if ( ch->stance == i )
    {
	send_to_char("You are already in that stance.\n\r", ch);
	return;
    }

    ch->stance = i;
        
    if ( !IS_NPC(ch) )
    if (get_eq_char( ch, WEAR_WIELD ) == NULL)
    {
	if (!stances[ch->stance].martial)
        {
	    send_to_char("You need a weapon for that.\n\r", ch);
	    ch->stance = 0;
	    return;
	}
    }
    else
    {
	if (!stances[ch->stance].weapon)
        {
	    send_to_char("You cant do that with a weapon.\n\r", ch);
	    ch->stance = 0;
	    return;
	}
    }

    if ( is_affected(ch, gsn_paroxysm) )
    {
        send_to_char("You're too hurt to do that right now.\n\r", ch);
        ch->stance = 0;
        return;
    }
        
    printf_to_char(ch, "You begin to fight in the %s style.\n\r", stances[i].name);
    /* show stance switch to other players */
    if ( ch->fighting != NULL )
    {
	char buf[MSL];
	sprintf( buf, "$n begins to fight in the %s style.", stances[i].name);
	act( buf, ch, NULL, NULL, TO_ROOM );
    }
}

/* Makes a simple percentage check against ch's skill level in the stance they
   are currently using.  */
bool check_lose_stance(CHAR_DATA *ch)
{
    int skill;

    if (ch->stance == 0)
        return FALSE;     /* Player not in a stance */

    skill = get_skill(ch, *(stances[ch->stance].gsn));
    
    if ( number_percent() > skill * 9/10 ) /* Always keep 10% chance */
        return TRUE;  /* Player loses their stance */

    return FALSE; /* Player keeps their stance */
}


CHAR_DATA* get_combat_victim( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( ch == NULL || argument == NULL )
	return NULL;

    if ( argument[0] == '\0' )
    {
	if ( ch->fighting == NULL )
	    send_to_char( "You aren't fighting anyone.\n\r", ch );
	return ch->fighting;
    }

    victim = get_char_room( ch, argument ); 

    if ( victim == NULL )
    {
	send_to_char( "You don't see them here.\n\r", ch );
	return NULL;
    }

    if ( is_safe(ch, victim) )
	return NULL;

    check_killer( ch, victim );

    return victim;
}

void adjust_pkgrade( CHAR_DATA *killer, CHAR_DATA *victim, bool theft )
{
	int grade_level;
	int earned;
	int lost;

	grade_level = get_pkgrade_level(victim->pcdata->pkpoints > 0 ?
					victim->pcdata->pkpoints : victim->pcdata->pkill_count);

	earned = pkgrade_table[grade_level].earned;
	lost = pkgrade_table[grade_level].lost;

	/* Immortals don't affect pkpoints .. also, a freaky safety net */
	if( IS_IMMORTAL(killer) || IS_NPC(killer) )
	    return;

	if( theft )
	{
	    /* Stealing, rather than killing, gets 1/10 the regular earned points */
	    earned = UMAX( (int)(.1*earned), 1 ); /* min of one  */
	    lost = (int)(.1*earned);             /* can be zero, and in fact IS zero until rank T */
	}

	if( IS_SET(killer->act, PLR_PERM_PKILL) )
	{
	    /* Declared pkillers receive their full share of the earnings. */
	    killer->pcdata->pkpoints += earned;

	    /* Victims without pkill on lose only 1 point per death (0 for theft) */
	    if( IS_SET(victim->act, PLR_PERM_PKILL) )
	        victim->pcdata->pkpoints -= lost;
	    else
	        victim->pcdata->pkpoints -= UMIN( 1, lost );
	}
	else
	{
	    /* People who have not declared themselves pkillers only earn 1 point per kill (0 for theft) */
	    killer->pcdata->pkpoints += UMIN( 1, earned );

	    /* Victims without pkill on lose only 1 point per death (0 for theft) */
	    if( IS_SET(victim->act, PLR_PERM_PKILL) )
	        victim->pcdata->pkpoints -= lost;
	    else
	        victim->pcdata->pkpoints -= UMIN( 1, lost );
	}
}

void adjust_wargrade( CHAR_DATA *killer, CHAR_DATA *victim )
{
	int grade_level;

	grade_level = get_pkgrade_level(victim->pcdata->warpoints > 0 ?
					victim->pcdata->warpoints : victim->pcdata->war_kills);

	killer->pcdata->warpoints += pkgrade_table[grade_level].earned;
	victim->pcdata->warpoints -= pkgrade_table[grade_level].lost_in_warfare;
}

int get_pkgrade_level( int pts )
{
    int grade_level;

    /* Grade A is grade_level 1.  Move down pkgrade_table until pkpoints <= pts. */

    for( grade_level=1; pkgrade_table[grade_level].pkpoints > 0; grade_level++ )
        if ( pkgrade_table[grade_level].pkpoints > pts )
            continue;
        else
            break;

    return grade_level;
}

