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
#include "special.h"

extern WAR_DATA war;

void reverse_char_list();
void check_rescue( CHAR_DATA *ch );
void check_jump_up( CHAR_DATA *ch );
void aura_damage( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield );
void stance_after_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield );
void weapon_flag_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield );
void check_behead( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield );
void handle_death( CHAR_DATA *ch, CHAR_DATA *victim );
void set_pos( CHAR_DATA *ch, int position );
void set_fighting_new( CHAR_DATA *ch, CHAR_DATA *victim, bool kill_trigger );
void adjust_pkgrade( CHAR_DATA *killer, CHAR_DATA *victim, bool theft );
void adjust_wargrade( CHAR_DATA *killer, CHAR_DATA *victim );

/* command procedures needed */
DECLARE_DO_FUN(do_fstat	    );
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
bool check_avoidance( CHAR_DATA *ch, CHAR_DATA *victim );
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
bool  check_shield  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void  dam_message   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
						 int dt, bool immune ) );
void  death_cry     args( ( CHAR_DATA *ch ) );
void  group_gain    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int   xp_compute    args( ( CHAR_DATA *gch, CHAR_DATA *victim, int gain_align ) );
bool  is_safe       args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void  make_corpse   args( ( CHAR_DATA *ch, CHAR_DATA *killer, bool to_morgue ) );
void  split_attack  args( ( CHAR_DATA *ch, int dt ) );
void  mob_hit       args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void  raw_kill      args( ( CHAR_DATA *victim, CHAR_DATA *killer, bool to_morgue ) );
void  set_fighting  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void  behead        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool  check_duck    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
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
bool is_normal_hit( int dt );
bool full_dam( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type, bool show );
bool deal_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type, bool show, bool lethal );
bool is_safe_check( CHAR_DATA *ch, CHAR_DATA *victim,
                    bool area, bool quiet, bool theory );
bool check_kill_steal( CHAR_DATA *ch, CHAR_DATA *victim );


bool check_critical(CHAR_DATA *ch, bool secondary)
{
    // need a weapon
    if ( !get_eq_char(ch, secondary ? WEAR_SECONDARY : WEAR_WIELD) )
        return false;
    
    // max chance is 5% critical skill + 5% critical mastery + 5% weapon mastery = max 15%
    if ( per_chance(85) )
        return FALSE;
    
    int weapon_sn = get_weapon_sn_new(ch, secondary);
    int weapon_mastery = get_mastery(ch, weapon_sn);
    int critical_mastery = get_mastery(ch, gsn_critical);    
    int chance = get_skill(ch, gsn_critical);

    if ( weapon_mastery )
        chance += 20 + 40 * weapon_mastery;
    if ( critical_mastery )
        chance += 20 + 40 * critical_mastery;
    
    return per_chance(chance / 3);
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
                mp_percent_trigger( ch, victim, NULL,0, NULL,0, TRIG_FIGHT );
            if (ch->must_extract) continue;

            if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
                mp_hprct_trigger( ch, victim );
            if (ch->must_extract) continue;

            if ( HAS_TRIGGER( ch, TRIG_MPCNT ) )
                mp_mprct_trigger( ch, victim );
            if (ch->must_extract) continue;
        }

        op_fight_trigger( ch, victim );
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

bool is_wimpy( CHAR_DATA *ch )
{
    if ( ch->hit < ch->max_hit * ch->wimpy/100 )
        return TRUE;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_WIMPY) && ch->hit < ch->max_hit / 2 )
        return TRUE;
    
    if ( IS_AFFECTED(ch, AFF_FEAR) )
        return TRUE;
    
    return FALSE;
}

/* execute a default combat action */
void run_combat_action( DESCRIPTOR_DATA *d )
{
    char *command;
    CHAR_DATA *ch;

    if ( d == NULL || (ch = d->character) == NULL || ch->pcdata == NULL )
        return;
    
    command = is_wimpy(ch) ? "flee" : ch->pcdata->combat_action;
    if ( command == NULL || command[0] == '\0' )
        return;

    /* no actions for charmed chars */
    if ( IS_AFFECTED(ch, AFF_CHARM) )
        return;

    anti_spam_interpret( d->character, command );

    /* prevent spam from lag-less actions */
    if ( ch->wait == 0 )
        WAIT_STATE( ch, PULSE_VIOLENCE/2 );
}

bool wants_to_rescue( CHAR_DATA *ch )
{
    if ( ch->position < POS_FIGHTING || is_wimpy(ch) )
        return FALSE;
    return (IS_NPC(ch) && IS_SET(ch->off_flags, OFF_RESCUE)) || PLR_ACT(ch, PLR_AUTORESCUE);
}

/* check if character rescues someone */
void check_rescue( CHAR_DATA *ch )
{
    CHAR_DATA *attacker, *target = NULL;
    char buf[MSL];

    if ( !wants_to_rescue(ch) )
        return;

    // get target
    for ( attacker = ch->in_room->people; attacker != NULL; attacker = attacker->next_in_room )
    {
        // may not be able to interfere
        if ( is_safe_spell(ch, attacker, FALSE) )
            continue;

        // may not want to rescue
        target = attacker->fighting;
        if ( target == NULL || target == ch || IS_NPC(target) || !is_same_group(ch, target) || !can_see_combat(ch, target) )
            continue;

        // may not want to be rescued
        if ( wants_to_rescue(target) )
            continue;
        
        break;
    }

    if ( attacker == NULL )
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
/*
bool save_body_affect( CHAR_DATA *ch, int level )
{
    int resist = ch->level * (100 + get_curr_stat(ch, STAT_VIT)) / 200;
    return number_range( 0, level ) < number_range( 0, resist );
}
*/

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
	    #ifdef FSTAT
	    ch->moves_used += (UMIN( ch->move, dam) );
	    #endif
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
			case SKY_CLOUDLESS: sunlight = 150; break;
			case SKY_CLOUDY: sunlight = 125; break;
			case SKY_RAINING: sunlight = 100; break;
			case SKY_LIGHTNING: sunlight = 50; break;
			default: sunlight = 100; break;
		}
		// reduce damage during sunrise
		if ( weather_info.sunlight != SUN_LIGHT )
                    sunlight /= 2;

		/* tune to fit char level */
		sunlight = (ch->level + 10) * sunlight/100;
        
		if ( !saves_spell(sunlight, ch, DAM_LIGHT) )
		{
			if ( IS_AFFECTED(ch, AFF_SHROUD))
			{
				sunlight /= 2;
				act_gag("Your shroud absorbs part of the sunlight.",ch,NULL,NULL,TO_CHAR,GAG_SUNBURN);
				full_dam( ch, ch, sunlight, gsn_torch, DAM_LIGHT, TRUE );
			}
			else
			{
				act_gag("The sunlight burns your skin.",ch,NULL,NULL,TO_CHAR,GAG_SUNBURN);
				/* misuse torch skill damage message */
				full_dam( ch, ch, sunlight, gsn_torch, DAM_LIGHT, TRUE );
			}
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
	af.modifier = 10 + ch->level;
	af.bitvector = AFF_BERSERK;
	
	af.location = APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location = APPLY_DAMROLL;
	af.modifier = 2*(10 + ch->level);
	affect_to_char(ch,&af);

	send_to_char( "{WYou're enraged with shan-ya battle madness!{x\n\r", ch);
	act( "{W$n {Wis enraged with shan-ya battle madness!{x", ch,NULL,NULL,TO_ROOM);
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
        int damage = number_range (5, 25) + ch->level*3/2;
        send_to_char( "Your festering wound oozes blood.\n\r", ch );
        deal_damage( ch, ch, damage, gsn_infectious_arrow, DAM_DISEASE, TRUE, FALSE );
    }

    /* Rupture - DOT - Damage over time */
    if ( is_affected(ch, gsn_rupture) )
    {
        int damage = number_range (5, 25) + ch->level*3/2;
        send_to_char( "Your ruptured wound oozes blood.\n\r", ch );
        deal_damage( ch, ch, damage, gsn_rupture, DAM_PIERCE, TRUE, FALSE );
    }

    /* Paralysis - DOT - Damage over time - Astark Oct 2012 */
    if ( IS_AFFECTED(ch, AFF_PARALYSIS) )
    {
        int damage = number_range (5, 25) + ch->level*3/2;
        send_to_char( "The paralyzing poison cripples you.\n\r", ch );
        deal_damage( ch, ch, damage, gsn_paralysis_poison, DAM_POISON, TRUE, FALSE );
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
                        if (can_see_combat(rch,vch)
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
            if ( ch->in_room == NULL )
            {
                bugf("stance_hit: ch->in_room NULL for %s", ch->name);
		break;
	    }

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
            if ( per_chance(15) )
                disarm(ch, victim, FALSE, get_mastery(ch, gsn_wendigo));
                   
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

// dual_axe, dual_sword, etc. used, or 0
int dual_weapon_sn( CHAR_DATA *ch )
{
    OBJ_DATA *wield = get_eq_char(ch, WEAR_WIELD);
    OBJ_DATA *second = get_eq_char(ch, WEAR_SECONDARY);
    
    if ( !wield || !second || wield->value[0] != second->value[0] )
        return 0;

    switch ( wield->value[0] )
    {
        case WEAPON_DAGGER: return gsn_dual_dagger;
        case WEAPON_SWORD:  return gsn_dual_sword;
        case WEAPON_AXE:    return gsn_dual_axe;
        case WEAPON_GUN:    return gsn_dual_gun;
        default:            return 0;
    }
}

/*
 * Effective skill for offhand weapon usage
 */
int dual_wield_skill( CHAR_DATA *ch, bool improve )
{
    OBJ_DATA *wield = get_eq_char(ch, WEAR_WIELD);
    OBJ_DATA *second = get_eq_char(ch, WEAR_SECONDARY);
    
    if ( wield == NULL || second == NULL )
        return 0;
    
    int wield_weight = UMAX(1, wield->weight);
    int second_weight = UMAX(1, second->weight);
    // cap off-hand weight to effectively cap penalty
    second_weight = UMIN(second_weight, 2*wield_weight);
    
    // dual wield requires weight difference
    int dual_wield = get_skill(ch, gsn_dual_wield);
    dual_wield = dual_wield * wield_weight / UMAX(wield_weight, second_weight * 3/2);
    
    if ( improve )
        check_improve(ch, gsn_dual_wield, TRUE, 10);
    
    // dual weapon requires weapons of correct type
    int dual_weapon = 0;
    int gsn_dual = dual_weapon_sn(ch);
    if ( gsn_dual > 0 )
    {
        dual_weapon = get_skill(ch, gsn_dual);
        // adjust for weight in case offhand weapon is heavier
        dual_weapon = dual_weapon * wield_weight / UMAX(wield_weight, second_weight);
        
        if ( improve )
            check_improve(ch, gsn_dual, TRUE, 8);
    }

    // combine the two skills, rounding down
    return dual_wield + (100 - dual_wield) * dual_weapon / 100;
}

/*
 * Chance for an offhand attack
 */
int offhand_attack_chance( CHAR_DATA *ch, bool improve )
{
    OBJ_DATA *wield = get_eq_char(ch, WEAR_WIELD);
    OBJ_DATA *second = get_eq_char(ch, WEAR_SECONDARY);
    bool hold = get_eq_char(ch, WEAR_HOLD) != NULL;
    bool shield = get_eq_char(ch, WEAR_SHIELD) != NULL;

    // unarmed attacks
    if ( wield == NULL )
    {
        if ( second == NULL && !hold && !shield )
            return (100 + 2 * UMIN(ch->level, 100)) / 3;
        else
            return 0;
    }

    // armed but no offhand weapon
    if ( second == NULL )
        return 0;

    // everybody has a base chance, regardless of skill
    int chance = (100 + 2 * dual_wield_skill(ch, improve)) / 3;

    if ( shield )
    {
        chance = chance * (100 + get_skill(ch, gsn_wrist_shield)) / 300;
        if ( improve )
            check_improve(ch, gsn_wrist_shield, TRUE, 20);
    }

    return chance;
}

/*
* Do one group of attacks.
*/
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int chance, mastery_chance, area_attack_sn;
    OBJ_DATA *wield;
    OBJ_DATA *second;

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
    second = get_eq_char ( ch, WEAR_SECONDARY );
    
    check_stance(ch);
    check_killer( ch, victim );
    
    /* automatic attacks for brawl & melee */
    if ( wield == NULL )
    {
        chance = get_skill(ch, gsn_brawl);
        area_attack_sn = gsn_brawl;
    }
    else
    {
        chance = get_skill(ch, gsn_melee) * 3/4;
        if ( wield->value[0] == WEAPON_POLEARM )
            chance += 25;
        area_attack_sn = gsn_melee;
    }
    mastery_chance = mastery_bonus(ch, area_attack_sn, 30, 50);

    if ( per_chance(chance) )
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
                // chance for extra (offhand if possible) attack
                if ( per_chance(mastery_chance) )
                {
                    if ( !wield || second )
                        one_hit(ch, vch, dt, TRUE);
                    else if ( number_bits(1) )
                        one_hit(ch, vch, dt, FALSE);
                }
            }
        }
        /* improve skill */
        if ( found )
            check_improve(ch, area_attack_sn, TRUE, 3);
    }

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
    
    if ( IS_AFFECTED(ch, AFF_HASTE) )
    {
        one_hit(ch,victim,dt,FALSE);
        if (ch->fighting != victim)
            return;
    }

    chance = offhand_attack_chance(ch, TRUE);
    
    if ( per_chance(chance) )
    {
        one_hit(ch, victim, dt, TRUE);
        if ( ch->fighting != victim )
            return;
        
        int gsn_dual = dual_weapon_sn(ch);
        int mastery = get_mastery(ch, gsn_dual_wield) + (gsn_dual ? get_mastery(ch, gsn_dual) : 0);
        chance = ch_dex_extrahit(ch) + (mastery ? 5 + 10*mastery : 0);
        if ( per_chance(chance) )
        {
            one_hit(ch, victim, dt, TRUE);
            if ( ch->fighting != victim )
                return;
        }
        
        if ( per_chance(get_skill(ch, gsn_extra_attack)/3) )
        {
            one_hit(ch,victim,dt,TRUE);
            if (ch->fighting != victim)
                return;
        }

        if ( IS_AFFECTED(ch,AFF_HASTE) && number_bits(1) == 0 )
        {
            one_hit(ch,victim,dt,TRUE);
            if (ch->fighting != victim)
                return;
        }

        if ( second != NULL && second->value[0] == WEAPON_DAGGER && number_bits(4) == 0 )
        {
            one_hit(ch,victim,dt,TRUE);
            if (ch->fighting != victim)
                return;
        }
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
        // chance for extra (offhand if possible) attack
        if ( per_chance(mastery_bonus(ch, gsn_ashura, 30, 50)) )
        {
            if ( !wield || second )
                one_hit(ch, victim, dt, TRUE);
            else if ( number_bits(1) )
                one_hit(ch, victim, dt, FALSE);
        }
    }
     
    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int i,chance,number;
    int attacks;
    CHAR_DATA *vch, *vch_next;
    
    if (ch->stop>0)
    {
	ch->stop--;
	return;
    }

    /* mobs must check their stances too */
    check_stance(ch);

    /* high level mobs get more attacks */
    attacks = level_base_attacks(ch->level);
    // note: this should match the calculation in mob_base_attacks (mob_stats.c)
    if ( IS_SET(ch->act, ACT_STAGGERED) )
        attacks = UMAX(100, attacks/2);    
    if ( IS_SET(ch->off_flags, OFF_FAST) )
        attacks = attacks * 3/2;
    if ( IS_AFFECTED(ch, AFF_GUARD) )
        attacks -= 50;
    if ( IS_AFFECTED(ch, AFF_HASTE) )
        attacks += 100;    
    if ( IS_AFFECTED(ch, AFF_SLOW) )
        attacks -= UMAX(0, attacks - 100) / 2;
    
    for ( ; attacks > 0; attacks -= 100 )
    {
        if (number_percent() > attacks)
            continue;
            
	one_hit(ch,victim,dt,FALSE);

	if (ch->fighting != victim)
	    return;
    }

    stance_hit(ch, victim, dt);
    CHECK_RETURN(ch, victim);
    
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
    
    if ( get_eq_char(ch, WEAR_SECONDARY) )
    {
        one_hit(ch,victim,dt, TRUE);
        if (ch->fighting != victim)
            return;
        // bonus off-hand attack for haste
        if ( IS_AFFECTED(ch,AFF_HASTE) && number_bits(1) )
        {
            one_hit(ch,victim,dt, TRUE);
            if (ch->fighting != victim)
                return;            
        }
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
	    if (IS_SET(ch->off_flags,OFF_DISARM))
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

int get_twohand_bonus( CHAR_DATA *ch, OBJ_DATA *wield, bool improve )
{
    if ( wield == NULL )
        return 0;
    
    bool has_shield = get_eq_char(ch, WEAR_SHIELD) != NULL;
    
    if ( wield->value[0] == WEAPON_BOW )
    {
        if ( has_shield )
        {
            if ( improve )
                check_improve(ch, gsn_wrist_shield, TRUE, 10);
            return get_skill(ch, gsn_wrist_shield);
        }
        else
            return 200;
    }
    
    if ( IS_WEAPON_STAT(wield, WEAPON_TWO_HANDS) )
    {
        int two_hand_bonus = (100 + get_skill(ch, gsn_two_handed)) / 2;
        int wrist_bonus = (100 + get_skill(ch, gsn_wrist_shield)) / 3;
        if ( improve )
            check_improve(ch, gsn_two_handed, TRUE, 10);
        if ( has_shield )
        {
            if ( improve )
                check_improve(ch, gsn_wrist_shield, TRUE, 10); 
            two_hand_bonus = two_hand_bonus * wrist_bonus / 100;
        }
        return two_hand_bonus;
    }

    // wielding a one-handed weapon in two hands
    if ( !has_shield && get_eq_char(ch, WEAR_SECONDARY) == NULL && get_eq_char(ch, WEAR_HOLD) == NULL )
        return 50;

    return 0;
}

/* returns the damage ch deals with one hit */
int one_hit_damage( CHAR_DATA *ch, int dt, OBJ_DATA *wield)
{
    int dam;

    /* basic damage */
    if ( IS_NPC(ch) )
	dam = dice(ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
    else
	dam = ch->level + dice( 2, 4 );

    /* weapon damage */
    if ( wield != NULL )
    {
        int weapon_dam = get_weapon_damage( wield );
        weapon_dam += weapon_dam * get_twohand_bonus(ch, wield, TRUE) / 100;
        dam += weapon_dam;
    }
    else
    {
	/* level 90+ bonus */
	if ( !IS_NPC(ch) && ch->level > (LEVEL_HERO - 10) )
	    dam += ch->level - (LEVEL_HERO - 10);
    }

    /* damage roll */
    int damroll = GET_DAMROLL(ch);
    if (damroll > 0) {
        int damroll_roll = number_range(0, number_range(0, damroll));
        // bonus is partially capped
        int damroll_cap = 2 * (10 + ch->level + UMAX(0, ch->level - 90));
        if (damroll_roll > damroll_cap)
            damroll_roll = (damroll_roll + 2 * damroll_cap) / 3;        
        dam += damroll_roll;
    }

    /* enhanced damage */
    if ( is_ranged_weapon(wield) )
    {
        int chance = get_skill(ch, gsn_sharp_shooting) / 2 + mastery_bonus(ch, gsn_sharp_shooting, 15, 25);
        if ( dt != gsn_burst && dt != gsn_semiauto && dt != gsn_fullauto
            && !number_bits(2) && per_chance(chance) )
        {
            dam *= 2;
            check_improve (ch, gsn_sharp_shooting, TRUE, 5);
        }
    }
    else
    {
        // enhanced damage mastery increases bonus damage
        dam += ch->level * (get_skill(ch, gsn_enhanced_damage) + mastery_bonus(ch, gsn_enhanced_damage, 30, 50)) / 300;
        check_improve (ch, gsn_enhanced_damage, TRUE, 10);
        dam += ch->level * get_skill(ch, gsn_brutal_damage) / 300;
        check_improve (ch, gsn_brutal_damage, TRUE, 10);
    }

    /* special attacks */
    if ( dt == gsn_backstab || dt == gsn_back_leap || dt == gsn_snipe ) 
	dam *= 3; 
    else if ( dt == gsn_aim || dt == gsn_circle || dt == gsn_slash_throat )
	dam *= 2;
    else if ( dt == gsn_parry )
	dam /= 2;

    /* anatomy */
    if ( (dt == gsn_backstab || dt == gsn_back_leap || dt == gsn_circle || dt == gsn_slash_throat) && chance(get_skill(ch, gsn_anatomy)) )
    {
        if ( wield != NULL && wield->value[0] == WEAPON_DAGGER )
            dam += dam * (100 + mastery_bonus(ch, gsn_anatomy, 15, 25)) / 200;
        else
            dam += dam * (100 + mastery_bonus(ch, gsn_anatomy, 15, 25)) / 400;
        check_improve(ch, gsn_anatomy, TRUE, 1);
    }
    return number_range( dam * 2/3, dam );
}

int martial_damage( CHAR_DATA *ch, int sn )
{
    int dam = one_hit_damage( ch, sn, NULL );
    
    dam += dam * mastery_bonus(ch, sn, 15, 25) / 100;

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
    if ( is_ranged_weapon(obj) || victim->fighting != ch || IS_AFFECTED(victim, AFF_FLEE) )
	return;
    one_hit( victim, ch, TYPE_UNDEFINED, FALSE );
}

int get_leadership_bonus( CHAR_DATA *ch, bool improve )
{
    int bonus;
    
    if ( ch->leader == NULL || ch->leader == ch || ch->in_room != ch->leader->in_room )
        return 0;

    bonus = get_curr_stat( ch->leader, STAT_CHA ) - 50;
    bonus += get_skill( ch->leader, gsn_leadership );
    bonus += ch->leader->level - ch->level;

    if (improve)
        check_improve( ch->leader, gsn_leadership, TRUE, 14 );

    return bonus / 10;
}

bool is_ranged_weapon( OBJ_DATA *weapon )
{
    if ( !weapon || weapon->item_type != ITEM_WEAPON )
        return FALSE;
    
    return weapon->value[0] == WEAPON_BOW
        || weapon->value[0] == WEAPON_GUN;
}

bool is_calm( CHAR_DATA *ch )
{
    return ch->move <= ch->max_move * ch->calm/100;
}

bool deduct_move_cost( CHAR_DATA *ch, int cost )
{
    if ( ch->move < cost )
        return FALSE;

    bool was_calm = is_calm(ch);
    
    ch->move -= cost;
    #ifdef FSTAT
    ch->moves_used += cost;
    #endif
    
    if ( !was_calm && is_calm(ch) )
        send_to_char("Worn with fatigue, you calm down.\n\r", ch);

    return TRUE;
}

/*
* Hit one guy once.
*/
bool one_hit ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )
{
    OBJ_DATA *wield;
    int dam, dam_type, diceroll, sn, skill, offence_cost;
    bool result, arrow_used = FALSE, offence = FALSE;
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
        return FALSE;
    
    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
        return FALSE;
    
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
                return FALSE;
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
                return FALSE;
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

    // deal extra damage at the cost of moves
    // the move cost applies whether or not the attack hits
    // that's why we check it here rather than in deal_damage
    if ( is_normal_hit(dt) && !is_calm(ch) )
    {
        offence_cost = 2;
        if ( wield != NULL )
        {
            if ( wield->value[0] == WEAPON_BOW )
                offence_cost = 5;
            else if ( IS_WEAPON_STAT(wield, WEAPON_TWO_HANDS) )
                offence_cost = 4;
            else
                offence_cost = 3;
        }
        // reduced cost for burst/semi-/full-auto
        if ( dt == gsn_burst || dt == gsn_semiauto || dt == gsn_fullauto )
            offence_cost = 2;
        // half cost while not berserking (but less damage later)
        if ( !IS_AFFECTED(ch, AFF_BERSERK) )
            offence_cost = rand_div(offence_cost, 2);
        offence = deduct_move_cost(ch, offence_cost);
    }
    
    start_combat(ch, victim);
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
        return FALSE;
    }
    
    // Check for parry, dodge, etc. and fade
    if ( is_normal_hit(dt) && check_avoid_hit(ch, victim, TRUE) )
        return FALSE;
        
    if (sn != -1)
	check_improve( ch, sn, TRUE, 10 );

    /*
     * Hit.
     * Calc damage.
     */
    dam = one_hit_damage( ch, dt, wield );
    
    if ( offence )
    {
        int bonus_fixed = offence_cost * 2;
        int bonus_percent = 15;
        // more damage for higher cost
        if ( IS_AFFECTED(ch, AFF_BERSERK) )
        {
            bonus_percent += 10 + mastery_bonus(ch, gsn_berserk, 3, 5);
            if ( per_chance(get_skill(ch, gsn_fervent_rage)) )
                bonus_percent += 10;
            check_improve(ch, gsn_fervent_rage, TRUE, 10);
        }
        dam += bonus_fixed + dam * bonus_percent/100;
    }

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
    
    // extra damage from smite attacks
    if ( dt == gsn_smite )
    {
        int align_diff = ABS(ch->alignment - victim->alignment);
        int skill = get_skill(ch, gsn_smite);
        dam += dam * align_diff / 2000 * skill / 100;
    }

    if ( IS_AFFECTED(ch, AFF_WEAKEN) )
        dam -= dam / 10;
    else if ( IS_AFFECTED(ch, AFF_GIANT_STRENGTH) )
        dam += dam / 20;
    
    if ( IS_AFFECTED(ch, AFF_POISON) )
        dam -= dam / 20;
    if ( IS_AFFECTED(ch, AFF_PLAGUE) )
        dam -= dam / 20;

    if ( check_critical(ch,secondary) == TRUE )
    {
	act("$p {RCRITICALLY STRIKES{x $n!",victim,wield,NULL,TO_NOTVICT);
        act("{RCRITICAL STRIKE!{x",ch,NULL,victim,TO_VICT);
        check_improve(ch,gsn_critical,TRUE,4);
        dam *= 2;
    }

    /* leadership and charisma of group leader */
    dam += dam * get_leadership_bonus(ch, TRUE) / 100;
    
    if ( dam <= 0 )
	dam = 1;
    
    result = full_dam( ch, victim, dam, dt, dam_type, TRUE );
    
    /* arrow damage & handling */
    if ( arrow_used )
	handle_arrow_shot( ch, victim, result );

    if ( stop_attack(ch, victim) )
        return result != 0;

    /* if not hit => no follow-up effects.. --Bobble */
    if ( !result )
        return FALSE;
    
    /* funky weapons */
    weapon_flag_hit( ch, victim, wield );
    if ( stop_attack(ch, victim) )
        return TRUE;

    /* behead */
    check_behead( ch, victim, wield );
    if ( stop_attack(ch, victim) )
        return TRUE;

    /* aura */
    aura_damage( ch, victim, wield );
    if ( stop_attack(ch, victim) )
        return TRUE;
    
    /* stance effects */
    stance_after_hit( ch, victim, wield );
    if ( stop_attack(ch, victim) )
        return TRUE;

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
    
    /* kung fu mastery */
    if ( !wield && is_normal_hit(dt) && per_chance(mastery_bonus(ch, gsn_kung_fu, 12, 20)) )
    {
        act_gag("You follow up with a flurry of blows!", ch, NULL, victim, TO_CHAR, GAG_WFLAG);
        one_hit(ch, victim, dt, secondary);
    }

   tail_chain( );
   return TRUE;
}

bool check_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam_type, int skill )
{
    int ch_roll, victim_roll;
    int victim_ac;
    int ac_dam_type;
    OBJ_DATA *wield;

    if ( ch == victim )
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_CURSE) && per_chance(5) )
	return FALSE;
    
    /* size */
    if ( number_percent() <= 3 * (ch->size - victim->size) )
	return FALSE;

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

    /* basic values */
    ch_roll = GET_HITROLL(ch);
    victim_roll = 10 - victim_ac;

    /* special skill adjustment */
    if ( (dt < TYPE_HIT && IS_SPELL(dt))
        || dt == gsn_aim
        || dt == gsn_backstab
        || dt == gsn_back_leap
        || dt == gsn_circle
        || dt == gsn_slash_throat
        || dt == gsn_rupture
        || dt == gsn_snipe )
    {
        victim_roll /= 2;
    }    
    else if ( dt == gsn_fullauto
        || dt == gsn_semiauto
        || dt == gsn_burst )
    {
        ch_roll = ch_roll * (10 + mastery_bonus(ch, dt, 4, 5)) / 25;
    }    
    
    if ( victim->size > ch->size )
    {
        // +5% to attack roll per size difference
        ch_roll += ch_roll * (victim->size - ch->size) * get_skill(ch, gsn_giantfeller) / 2000;
        check_improve(ch, gsn_giantfeller, TRUE, 6);
    }

    /* skill-based chance-to-miss */
    ch_roll = ch_roll * skill/100;

    /* blind attacks */
    if ( !can_see_combat( ch, victim ) && blind_penalty(ch) )
	ch_roll = ch_roll * 3/4;

    /* bad combat position */
    if ( ch->position < POS_FIGHTING )
	ch_roll = ch_roll * 3/4;

    /* special stance */
    if ( ch->stance == STANCE_DIMENSIONAL_BLADE && is_normal_hit(dt) )
	 ch_roll *= 2;

    if ( ch_roll <= 0 )
        return FALSE;
    if ( victim_roll <= 0 )
        return TRUE;

    int ch_rolled = number_range(0, ch_roll);
    int victim_rolled = number_range(0, victim_roll);
    bool is_hit = (ch_rolled > victim_rolled);

#ifdef TESTER
    if ( !IS_SET(ch->gag, GAG_MISS) )
        printf_to_char( ch, "Attack Roll (%d) = %d vs %d = Defense Roll (%d) => %s\n\r",
            ch_roll, ch_rolled, victim_rolled, victim_roll, is_hit ? "hit" : "miss" );
#endif

    return is_hit;
}

void aura_damage( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield )
{
    if ( !IS_AFFECTED(victim, AFF_ELEMENTAL_SHIELD) || is_ranged_weapon(wield) )
        return;

    AFFECT_DATA *aff = affect_find_flag(victim->affected, AFF_ELEMENTAL_SHIELD);

    if ( !aff )
    {
        bugf("aura_damage: no aura affect found on %s (%d).", victim->name, IS_NPC(victim) ? victim->pIndexData->vnum : 0);
        return;
    }
    
    int dam_type;
    int aff_sn = aff->type;
    int aff_level = aff->level;

    // quirky's insanity is special
    if ( aff_sn == gsn_quirkys_insanity )
    {
        if ( number_bits(2) == 0 )
        {
            int sn = per_chance(10) ? skill_lookup("confusion") : skill_lookup("laughing fit");
            (*skill_table[sn].spell_fun) (sn, aff_level/2, victim, (void*)ch, TARGET_CHAR);
        }
        full_dam(victim, ch, aff_level/2, gsn_quirkys_insanity, DAM_MENTAL, TRUE);
        return;
    }
    
    // now the "regular" elemental auras
    if ( aff_sn == gsn_immolation )
        dam_type = DAM_FIRE;
    else if ( aff_sn == gsn_epidemic )
        dam_type = DAM_DISEASE;
    else if ( aff_sn == gsn_electrocution )
        dam_type = DAM_LIGHTNING;
    else if ( aff_sn == gsn_absolute_zero )
        dam_type = DAM_COLD;
    
    int dam = aff_level;

    // save for half damage is possible but harder than normal
    if ( is_affected(ch, gsn_fervent_rage) || saves_spell(2*aff_level, ch, dam_type) )
        dam /= 2;

    full_dam(victim, ch, dam, aff_sn, dam_type, TRUE);
}

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
	    disarm(ch, victim, FALSE, get_mastery(ch, gsn_boa));
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
        if ( (IS_NPC(victim) && (IS_SET(victim->act,ACT_MAGE) || victim->spec_fun == spec_cast_mage))
             || (!IS_NPC(victim) && (victim->class == 3 || victim->class == 11 || victim->class == 14)) )
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
    if ( IS_WEAPON_STAT(wield, WEAPON_STORMING) && !number_bits(4) )
    {
        dam = 50 + dice(20,6);
        switch ( number_range(1,4) )
        {
            case 1: full_dam(ch, victim, dam, gsn_lightning_bolt, DAM_LIGHTNING, TRUE); break;
            case 2: full_dam(ch, victim, dam, gsn_meteor_swarm, DAM_FIRE, TRUE); break;
            case 3: full_dam(ch, victim, dam, gsn_monsoon, DAM_DROWNING, TRUE); break;
            case 4: full_dam(ch, victim, dam, gsn_hailstorm, DAM_COLD, TRUE); break;
        }
        CHECK_RETURN(ch, victim);
    }

    /* remove temporary weapon flags 
     * also solves problem with old weapons with different flags
     */
    for ( flag = 0; weapon_type2[flag].name != NULL; flag++ )
    {
	int bit = weapon_type2[flag].bit;

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
    if ( number_bits(9) != 69 )
        return;
    
    // beheading mastery increases behead chance by up to factor 2, depending on victim's health
    int dam_taken = (victim->max_hit - victim->hit) * 100 / victim->max_hit;
    if ( number_bits(1) && !(per_chance(dam_taken) && per_chance(mastery_bonus(ch, gsn_beheading, 60, 100))) )
        return;

    if (IS_NPC(ch) && IS_SET(ch->in_room->area->area_flags, AREA_REMORT))
        return;

    if (IS_SET(victim->act, ACT_NOBEHEAD))
    {
        act("You try to cut $N's head off, but it won't budge!", ch, NULL, victim, TO_CHAR);
        act("$n tries to cut $N's head off, but it won't budge!", ch, NULL, victim, TO_ROOM);
        return;
    }

    if ( wield == NULL )
    {
        /* razor claw is active AND passive skill => mobs have it */
        if ( !IS_NPC(ch) && number_bits(1) && per_chance(get_skill(ch, gsn_razor_claws))
             || ch->stance == STANCE_SHADOWCLAW )
        {
            act("In a mighty strike, your claws separate $N's neck.", ch, NULL, victim, TO_CHAR);
            act("In a mighty strike, $n's claws separate $N's neck.", ch, NULL, victim, TO_NOTVICT);
            act("$n slashes $s claws through your neck.", ch, NULL, victim, TO_VICT);
            behead(ch, victim);
        }
        return;
    }

    int chance = 0;
    switch ( wield->value[0] )
    {
    case WEAPON_EXOTIC: chance = 0; break;
    case WEAPON_DAGGER:
    case WEAPON_POLEARM: chance = 1; break;
    case WEAPON_SWORD: chance = 5; break;
    case WEAPON_AXE: chance = 25; break;
    default: return;
    }
   
    chance += get_skill(ch, gsn_beheading) / 2;
    if ( IS_WEAPON_STAT(wield, WEAPON_SHARP) ) 
        chance += 1;
    if ( IS_WEAPON_STAT(wield, WEAPON_VORPAL) )
        chance += 5;

    if ( per_chance(chance) || ch->stance == STANCE_SHADOWCLAW )
    {
        act("$n's head is separated from his shoulders by $p.", victim,wield,NULL,TO_ROOM);
        act("Your head is separated from your shoulders by $p.", victim,wield,NULL,TO_CHAR);
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

    // assassination mastery increases chance by up to factor 2, depending on victim's health
    int dam_taken = (victim->max_hit - victim->hit) * 100 / victim->max_hit;
    if ( per_chance(dam_taken) && per_chance(mastery_bonus(ch, gsn_beheading, 60, 100)) )
        chance = UMAX(0, chance - 1);
    
    extra_chance = 50 + (get_skill(ch, gsn_anatomy) + mastery_bonus(ch, gsn_anatomy, 15, 25)) / 4;
    if ( IS_WEAPON_STAT(wield, WEAPON_VORPAL) )
	extra_chance += 10;

    if ( number_bits(chance) == 0
	 && number_percent() <= extra_chance
	 && (ch->stance == STANCE_AMBUSH || number_bits(1))
	 && number_percent() <= get_skill(ch, gsn_assassination) )
    {
        if (IS_SET(victim->act, ACT_NOBEHEAD))
        {
            act("You try to cut $N's head off, but it won't budge!",
                ch,NULL,victim,TO_CHAR);
            act("$n tries to cut $N's head off, but it won't budge!",
                ch,NULL,victim,TO_ROOM);
            return;
        }
        else
        {
	    act("You sneak up behind $N, and slash $S throat!",ch,NULL,victim,TO_CHAR);
	    act("$n sneaks up behind you and slashes your throat!",ch,NULL,victim,TO_VICT);
	    act("$n sneaks up behind $N, and slashes $S throat!",ch,NULL,victim,TO_NOTVICT);
	    behead(ch,victim);
	    check_improve(ch,gsn_assassination,TRUE,0);
        }
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
    if ( IS_SET(ch->form, FORM_CONDUCTIVE) && dam_type == DAM_LIGHTNING )
        dam += dam/4;

    // vitality no longer affects immunities directly
    dam -= dam * (get_curr_stat(victim, STAT_VIT) - 100) / 1000;
    
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

/* strip affects due to dealing damage */
void attack_affect_strip( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( victim == ch )
        return;
    
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
        affect_strip( ch, gsn_invis );
        affect_strip( ch, gsn_mass_invis );
        REMOVE_BIT( ch->affect_field, AFF_INVISIBLE );
        act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }
    
    if ( IS_AFFECTED(ch, AFF_ASTRAL) )
    {
        affect_strip( ch, gsn_astral );
        REMOVE_BIT( ch->affect_field, AFF_ASTRAL );
        act( "$n returns to the material plane.", ch, NULL, NULL, TO_ROOM );
    }
    
    if ( IS_AFFECTED(ch, AFF_SHELTER) )
    {
        affect_strip( ch, gsn_shelter );
        REMOVE_BIT( ch->affect_field, AFF_SHELTER );
        act( "$n is no longer sheltered!", ch, NULL, NULL, TO_ROOM );
    }
    
    if ( IS_AFFECTED(ch, AFF_HIDE) )
    {
        affect_strip( ch, gsn_hide );
        REMOVE_BIT( ch->affect_field, AFF_HIDE );
        act( "$n leaps out of hiding!", ch, NULL, NULL, TO_ROOM );
    }

    // Followers desert (strips charm)
    if ( victim->master == ch )
        stop_follower( victim );
}

/* deal direct, unmodified, non-lethal damage */
void direct_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int sn )
{
    dam = URANGE( 0, dam, victim->hit - 1 );
    victim->hit -= dam;

    remember_attack(victim, ch, dam);

    if ( dam > 0 )
    {
        // Sleeping victims wake up
        if ( IS_AFFECTED(victim, AFF_SLEEP) )
            affect_strip_flag( victim, AFF_SLEEP );
        if ( victim->position == POS_SLEEPING )
            set_pos( victim, POS_RESTING );
        
        if ( sn > 0 )
            dam_message(ch,victim,dam,sn,FALSE);
    }
}

/*
* Inflict reduced damage from a hit.
*/
bool damage( CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,
	     bool show )
{
    // damage reduction
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

    return deal_damage( ch, victim, dam, dt, dam_type, show, TRUE );
}

// if ch is a charmed NPC and leader is present, returns leader, otherwise ch
CHAR_DATA *get_local_leader( CHAR_DATA *ch )
{
    if ( ch != NULL && IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->leader != NULL && ch->leader->in_room == ch->in_room )
        return ch->leader;
    else
        return ch;
}

bool check_evasion( CHAR_DATA *ch, CHAR_DATA *victim, int sn, bool show )
{
    int chance = get_skill(victim, gsn_evasion);
    
    // need skill to evade
    if ( !chance )
        return FALSE;
    
    // touch spells and some others cannot be evaded
    char *spell = skill_table[sn].name;
    if ( !strcmp(spell, "burning hands")
        || !strcmp(spell, "chill touch")
        || !strcmp(spell, "shocking grasp")
        || !strcmp(spell, "poison")
        || !strcmp(spell, "plague")
        || !strcmp(spell, "iron maiden"))
        return FALSE;

    // direct-target spells are harder to evade
    if ( skill_table[sn].target != TAR_IGNORE && !per_chance(mastery_bonus(victim, gsn_evasion, 30, 50)) )
        chance /= 2;

    bool success = per_chance(chance);
    if ( show && success )
    {
        act_gag("$N evades your spell, reducing its impact.", ch, NULL, victim, TO_CHAR, GAG_MISS);
        act_gag("You evade $n's spell, reducing its impact.", ch, NULL, victim, TO_VICT, GAG_MISS);
        act_gag("$N evades $n's spell, reducing its impact.", ch, NULL, victim, TO_NOTVICT, GAG_MISS);
    }
    check_improve(victim, gsn_evasion, success, 5);
    return success;
}

/*
* Inflict full damage from a hit.
*/
bool full_dam( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type, bool show )
{
    return deal_damage(ch, victim, dam, dt, dam_type, show, TRUE);
}

bool deal_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type, bool show, bool lethal )
{
    bool immune;
    char buf[MAX_STRING_LENGTH];
    int stance = ch->stance;
    int diff;
    
    if ( stop_attack(ch, victim) )
        return FALSE;
    
    if ( is_safe(ch, victim) )
        return FALSE;

    /*
    * Stop up any residual loopholes.
    */
    if ( dam > 2000 && dt >= TYPE_HIT && !IS_IMMORTAL(ch))
    {
        OBJ_DATA *weapon = get_eq_char(ch, WEAR_WIELD);
        OBJ_DATA *offhand = get_eq_char(ch, WEAR_SECONDARY);
        int weapon_dam = weapon ? average_weapon_dam(weapon) : 0;
        int offhand_dam = offhand ? average_weapon_dam(offhand) : 0;
        bugf("Excessive Damage: %d points (weapon avg = %d) from a regular hit by %s!", dam, UMAX(weapon_dam, offhand_dam), ch->name);
    }
    
    if ( victim != ch )
    {
        
        check_killer(ch, victim);
        if ( !start_combat(ch, victim) )
            return FALSE;
        
        if ( victim->position > POS_STUNNED )
        {
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

    if ( dam > 1 && dt > 0 && dt < TYPE_HIT && IS_SPELL(dt) )
    {
        if ( victim->stance == STANCE_ARCANA )
            dam -= dam/3;
        if ( check_evasion(ch, victim, dt, show) )
            dam -= dam/2;
    }

    immune = FALSE;
    
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
    else if ( victim->stance == STANCE_TORTOISE
            || victim->stance == STANCE_AVERSION )
	    dam -= dam / 3;
	else if ( victim->stance == STANCE_WENDIGO )
	    dam -= dam / 5;
    }

    /* religion bonus */
    /*
    if ( ch != victim )
	dam += dam * get_religion_bonus(ch) / 100;
    */

    // non-spell damage may be reduced by a saving throw as well
    if ( dam > 1 && is_normal_hit(dt) )
    {
        int victim_roll = -get_save(victim, TRUE);
        int ch_roll = 2 * (10 + ch->level) + get_hitroll(ch);
        if ( number_range(0,ch_roll) < number_range(0,victim_roll) )
            dam /= 2;
    }
    
    // stone skin reduce damage but wears off slowly
    if ( dam > 1 && IS_AFFECTED(victim, AFF_STONE_SKIN) )
    {
        AFFECT_DATA *aff = affect_find_flag(victim->affected, AFF_STONE_SKIN);
        int level = (aff ? aff->level : victim->level);
        int max_reduction = 10 + level/4;
        int reduction = URANGE(1, dam/10, max_reduction);
        dam -= reduction;
        // chance to reduce duration
        if ( aff && aff->duration > 0 && number_range(1,max_reduction) <= reduction )
            aff->duration -= 1;
    }
    
    if (dt == gsn_beheading)
    {
        immune = FALSE;
        dam = victim->hit + 100;
    }
    
    if (show)
        dam_message( ch, victim, dam, dt, immune );
    
    if (dam == 0)
    {
	#ifdef FSTAT
	ch->attacks_misses += 1;
	#endif
        return FALSE;
    }
    #ifdef FSTAT
    ch->attacks_success += 1;
    #endif
    
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
		//else dam -= (victim->hit/victim->max_hit - .25) * 2/3; Quirky, Quirky.. :P
		else dam = dam * victim->hit/victim->max_hit * 2/3 - dam/6;
	    }
	}

	if ( is_affected(victim, gsn_mana_shield) )
	{
	    int mana_loss = UMIN(dam / 2, victim->mana);
	    victim->mana -= mana_loss;
	    dam -= mana_loss;
	}
    }

    int grit = get_skill(victim, gsn_true_grit);
    if ( grit > 0 && dam > 0 && lethal && dt != gsn_beheading && victim->move > 0 )
    {
        // absorb only damage that would drop victim below 1 hp
        int max_absorb = dam - UMAX(0, victim->hit - 1);
        int absorb = URANGE(0, max_absorb, victim->move);
        if ( absorb > 0 )
        {
            int absorb_roll = number_range(0, absorb);
            int grit_max = (victim->move << get_mastery(victim, gsn_true_grit)) * grit/100;
            int grit_roll = number_range(0, grit_max);
            #ifdef TESTER
            printf_to_char(victim, "True Grit: absorb-roll(%d) = %d vs %d = grit-roll(%d)\n\r", absorb, absorb_roll, grit_roll, grit_max);
            #endif
            if ( grit_roll > absorb_roll )
            {
                victim->move -= absorb;
                victim->hit += absorb;
                if ( show && !IS_SET(victim->gag, GAG_BLEED) )
                    send_to_char("You cling to life, showing true grit!\n\r", victim);
                check_improve(victim, gsn_true_grit, TRUE, 1);
            }
            else
                check_improve(victim, gsn_true_grit, FALSE, 0);
        }
    }
    
    if (lethal)
        victim->hit -= dam;
    else if (victim->hit > 0)
        victim->hit = UMAX(1, victim->hit - dam);
        
    #ifdef FSTAT 
    victim->damage_taken += dam;
    ch->damage_dealt += dam;
    if ( dam < 1 )
	ch->attacks_misses +=1;
    #endif
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
        // suicide during combat counts as kill by opponent
        if ( ch == victim && victim->fighting != NULL )
            ch = victim->fighting;
        if (victim!=ch || !(!IS_NPC(victim) && IS_SET(victim->act, PLR_WAR)))
        {
            
            act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
            send_to_char( "You have been KILLED!!\n\r\n\r", victim );
            
            if (!IS_NPC(victim) && !IS_SET( victim->act, PLR_WAR)) 
            {
                CHAR_DATA *killer = get_local_leader(ch);
                if( IS_NPC(killer) )
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
                victim->hit = 1;
                set_pos( victim, POS_STUNNED );
                // trigger must come AFTER death-prevention, as mob remort can cause character to save
                mp_percent_trigger( m, victim, NULL,0, NULL,0, TRIG_DEFEAT );
                return FALSE;
            }
    }
    
    /*
    * Payoff for killing things.
    */
    if ( victim->position == POS_DEAD )
    {
        handle_death( get_local_leader(ch), victim );
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
    {
        forget_attacker(ch, victim);
        if ( ch->hunting && !str_cmp(ch->hunting, victim->name) )
            stop_hunting(ch);
    }
        
    if ( !PLR_ACT(ch, PLR_WAR) )
	group_gain( ch, victim );

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	ch->pcdata->mob_kills++;
	update_lboard( LBOARD_MKILL, ch, ch->pcdata->mob_kills, 1);
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
		update_lboard( LBOARD_PKILL, ch, ch->pcdata->pkill_count, 1);
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
	else if (IS_NPC(ch) && (ch->spec_fun == spec_bounty_hunter)
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
        mp_percent_trigger( victim, ch, NULL,0, NULL,0, TRIG_DEATH );
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
#define PKILL_RANGE 6
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

           /* no pets unless you could attack their owner */
            if (IS_AFFECTED(victim, AFF_CHARM) && victim->leader != NULL && victim->leader != victim)
            {
                bool is_safe = is_safe_check(ch, victim->leader, area, TRUE, TRUE);
                if (is_safe && !quiet)
                    send_to_char("You don't own that monster.\n\r",ch);
                return is_safe;
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
            int level_offset = PKILL_RANGE;
            int ch_power = ch->level + 2 * ch->pcdata->remorts;
            int victim_power = victim->level + 2 * victim->pcdata->remorts;
            
            clanwar_valid = is_clanwar_opp(ch, victim);
            /* || is_religion_opp(ch, victim); */

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
                    printf_to_char(ch, "%s would frown upon that.\n\r", clan_table[ch->clan].patron);
                return TRUE;
            }
            */
            
            if (IS_SET(victim->act,PLR_KILLER))
                level_offset += 2;
            
            if (IS_SET(victim->act, PLR_THIEF))
                level_offset += 3;
            
            if (ch_power > victim_power + level_offset)
            {
                if ( !quiet )
                    send_to_char("Pick on someone your own size.\n\r",ch);
                return TRUE;
            }
            
            /* This was added to curb the ankle-biters. Rim 3/15/98 */
            level_offset = PKILL_RANGE;
            
            if (IS_SET(ch->act,PLR_KILLER))
                level_offset += 2;
            
            if (IS_SET(ch->act, PLR_THIEF))
                level_offset += 3;

            if (victim_power > ch_power + level_offset)
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
    bool finesse, autohit, try_avoid;
    int stance = ch->stance;
    int vstance = victim->stance;
    int sector = ch->in_room->sector_type;

    if ( ch == victim )
	return FALSE;
    
    /* only normal attacks can be faded, no spells */
    if ( check_fade( ch, victim, show ) )
	return TRUE;
    
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

    try_avoid = !autohit && (vstance == STANCE_BUNNY || !(finesse && number_bits(1) == 0)) && !IS_AFFECTED(victim, AFF_FLEE);
    if ( try_avoid )
    {
        if ( check_outmaneuver( ch, victim ) )
            return TRUE;
        if ( check_avoidance( ch, victim ) )
            return TRUE;
        if ( check_duck( ch,victim ) )
            return TRUE;
        if ( check_dodge( ch, victim ) )
            return TRUE;
    }

    if ( check_mirror( ch, victim, show ) )
        return TRUE;
    if ( check_phantasmal( ch, victim, show ) )
        return TRUE;

    if ( check_shield(ch, victim) )
        return TRUE;
    if ( check_shield_block(ch,victim) )
        return TRUE;

    if ( try_avoid && check_parry( ch, victim ) )
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
    
    // allow disbelief against illusion
    if ( per_chance(get_skill(ch, gsn_alertness)) || number_bits(2) )
    {
        check_improve(ch, gsn_alertness, TRUE, 3);
        if ( saves_spell(aff->level, ch, DAM_MENTAL) )
            return FALSE;
    }
    else
        check_improve(ch, gsn_alertness, FALSE, 1);

    // might still hit caster by pure chance
    if ( number_range(0, aff->bitvector) == 0 )
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

    // allow disbelief against illusion
    if ( per_chance(get_skill(ch, gsn_alertness)) || number_bits(2) )
    {
        check_improve(ch, gsn_alertness, TRUE, 3);
        if ( saves_spell(aff->level, ch, DAM_MENTAL) )
            return FALSE;
    }
    else
        check_improve(ch, gsn_alertness, FALSE, 1);

    // might still hit caster by pure chance
    if ( number_range(0, aff->bitvector) == 0 )
        return FALSE;

    /* ok, we hit a phantasmal image */
    if ( show )
    {
        act_gag( "$n hits one of your phantasmal images, which explodes in a flash of light.",
		 ch, NULL, victim, TO_VICT, GAG_FADE );
        act_gag( "You hit one of $N's phantasmal images, which explodes in a flash of light.",
		 ch, NULL, victim, TO_CHAR, GAG_FADE );
        act_gag( "$n hits one of $N's phantasmal images, which explodes in a flash of light.",
		 ch, NULL, victim, TO_NOTVICT, GAG_FADE );

        // like auras, ranged attackers don't take damage from exploding images
        int ch_weapon = get_weapon_sn(ch);
        if ( ch_weapon != gsn_gun && ch_weapon != gsn_bow )
        {
            dam = dice(4, aff->level);
            // allow hard save for half damage
            if ( saves_spell(2*aff->level, ch, DAM_LIGHT) )
                dam /= 2;
            full_dam(victim, ch, dam, gsn_phantasmal_image, DAM_LIGHT, TRUE);
        }
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

int parry_chance( CHAR_DATA *ch, CHAR_DATA *opp, bool improve )
{
    int gsn_weapon = get_weapon_sn(ch);

    if ( gsn_weapon == gsn_gun || gsn_weapon == gsn_bow )
        return 0;
    if ( gsn_weapon == gsn_hand_to_hand && !(IS_NPC(ch) && IS_SET(ch->off_flags, OFF_PARRY)) )
        return 0;

    int opponent_adjust = 0;
    if ( opp )
    {
        int level_diff = ch->level - opp->level;
        int stat_diff = get_curr_stat(ch, STAT_DEX) - get_curr_stat(opp, STAT_DEX);
        opponent_adjust = (level_diff + stat_diff/4) / 2;
    }
    int chance = 10 + (get_skill(ch, gsn_parry) + opponent_adjust) / 4;
    
    /* some weapons are better for parrying, some are worse */
    if ( gsn_weapon == gsn_sword )
        chance += 5;
    else if ( gsn_weapon == gsn_flail || gsn_weapon == gsn_whip )
        chance -= 5;

    if ( ch->stance == STANCE_SWAYDES_MERCY || ch->stance == STANCE_AVERSION )
        chance += 10;
    
    if ( IS_AFFECTED(ch, AFF_SORE) )
        chance -= 10;

    chance += mastery_bonus(ch, gsn_parry, 3, 5);
    
    if ( improve )
        check_improve(ch, gsn_parry, TRUE, 15);
    
    return URANGE(0, chance, 75);
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

    if ( ch_weapon == gsn_gun || ch_weapon == gsn_bow )
        return FALSE;

    if ( (chance = parry_chance(victim, ch, TRUE)) == 0 )
        return FALSE;
    
    /* some weapons are harder to parry */
    if ( ch_weapon == gsn_whip || ch_weapon == gsn_flail )
        chance -= 10;
    else if ( ch_weapon == gsn_hand_to_hand )
        chance -= mastery_bonus(ch, gsn_hand_to_hand, 6, 10);

    /* two-handed weapons are harder to parry with non-twohanded */
    if ( (ch_weapon_obj = get_eq_char(ch, WEAR_WIELD)) != NULL
	 && IS_WEAPON_STAT(ch_weapon_obj, WEAPON_TWO_HANDS) )
    {
	if ( (victim_weapon_obj = get_eq_char(victim, WEAR_WIELD)) == NULL
	     || !IS_WEAPON_STAT(victim_weapon_obj, WEAPON_TWO_HANDS) )
	    chance /= 2;
    }

    if ( !can_see_combat(ch,victim) && blind_penalty(victim) )
        chance /= 2;
    
    if ( !per_chance(chance) )
        return FALSE;
    
    act_gag( "You parry $n's attack.",  ch, NULL, victim, TO_VICT, GAG_MISS );
    act_gag( "$N parries your attack.", ch, NULL, victim, TO_CHAR, GAG_MISS );
    act_gag( "$N parries $n's attack.", ch, NULL, victim, TO_NOTVICT, GAG_MISS );

    /* whips can disarm or get disarmed on successfull parry */
    if ( ch_weapon == gsn_whip && number_bits(5) == 0 )
    {
	if ( victim_weapon != gsn_hand_to_hand && number_bits(1) )
	{
	    /* disarm */
	    act( "Your whip winds around $N's weapon.", ch, NULL, victim, TO_CHAR );
	    act( "$n's whip winds around your weapon.", ch, NULL, victim, TO_VICT );
	    act( "$n's whip winds around $N's weapon.", ch, NULL, victim, TO_NOTVICT );
	    disarm( ch, victim, FALSE, get_mastery(ch, gsn_whip) );
	}
	else
	{
	    /* get disarmed */
	    act( "Your whip winds around $N's arm.", ch, NULL, victim, TO_CHAR );
	    act( "$n's whip winds around your arm.", ch, NULL, victim, TO_VICT );
	    act( "$n's whip winds around $N's arm.", ch, NULL, victim, TO_NOTVICT );
	    disarm( victim, ch, FALSE, get_mastery(victim, gsn_disarm) );
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
	    dam = one_hit_damage(victim, gsn_parry, wield) * (100 - mastery_bonus(ch, gsn_hand_to_hand, 30, 50)) / 100;
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
    if ( !IS_AWAKE(victim) )
        return FALSE;
    
    if ( get_weapon_sn(ch) != gsn_gun && get_weapon_sn(ch) != gsn_bow )
        return FALSE;
    
    int skill = get_skill(victim,gsn_duck);
    
    if (skill == 0)
        return FALSE;

    int level_diff = victim->level - ch->level;
    int stat_diff = get_curr_stat(victim, STAT_AGI) - get_curr_stat(ch, STAT_DEX);
    int opponent_adjust = (level_diff + stat_diff/4) / 2;
    int chance = (skill + opponent_adjust) / 3;

    if (victim->stance==STANCE_SHOWDOWN)
        chance += 30;

    if ( IS_AFFECTED(victim, AFF_SORE) )
        chance -= 10;

    chance += mastery_bonus(ch, gsn_duck, 3, 5);

    chance = URANGE(0, chance, 75);
    
    if ( !can_see_combat(victim,ch) && blind_penalty(victim) )
        chance -= chance / 4;

    if ( !per_chance(chance) )
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

    chance += mastery_bonus(ch, gsn_mass_combat, 3, 5);

    chance = URANGE(0, chance, 75);

    if ( !can_see_combat(victim,ch) && blind_penalty(victim) )
        chance -= chance/4;

    if ( number_percent() > chance )
        return FALSE;
    
    act_gag( "You outmaneuver $n's attack!", ch, NULL, victim, TO_VICT, GAG_MISS );
    act_gag( "$N outmaneuvers your attack!", ch, NULL, victim, TO_CHAR, GAG_MISS );
    act_gag( "$N outmaneuvers $n's attack.", ch, NULL, victim, TO_NOTVICT, GAG_MISS );
    check_improve(victim,gsn_mass_combat,TRUE,15);
    return TRUE;
}

bool check_avoidance( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if ( ch->fighting == victim || ch->fighting == NULL )
        return FALSE;

    chance = get_skill(victim, gsn_avoidance) * 2/3;

    if ( IS_AFFECTED(victim, AFF_SORE) )
        chance -= 10;

    if ( !can_see_combat(victim,ch) && blind_penalty(victim) )
        chance -= chance/4;

    if ( !per_chance(chance) )
        return FALSE;

    act_gag( "You avoid $n's attack!", ch, NULL, victim, TO_VICT, GAG_MISS );
    act_gag( "$N avoids your attack!", ch, NULL, victim, TO_CHAR, GAG_MISS );
    act_gag( "$N avoids $n's attack.", ch, NULL, victim, TO_NOTVICT, GAG_MISS );
    check_improve(victim,gsn_avoidance,TRUE,5);
    return TRUE;
}

bool check_jam( CHAR_DATA *ch, int odds, bool offhand )
{
    OBJ_DATA *gun;
    
    if ( ch->stance == STANCE_SHOWDOWN && number_bits(2) )
        return FALSE;

    if ( odds < number_range(1, 1000) )
        return FALSE;
        
    gun = offhand ? get_eq_char(ch, WEAR_SECONDARY) : get_eq_char(ch, WEAR_WIELD);
    if ( !gun || gun->value[0] != WEAPON_GUN )
        return FALSE;
    
    SET_BIT(gun->extra_flags,ITEM_JAMMED);
    if ( offhand )
        send_to_char( "Your offhand gun is jammed!\n\r", ch );
    else
        send_to_char( "Your gun is jammed!\n\r", ch );

    return TRUE;
}

int shield_block_chance( CHAR_DATA *ch, bool improve )
{
    if ( get_eq_char(ch, WEAR_SHIELD) == NULL )
        return 0;

    int chance = 10 + get_skill(ch, gsn_shield_block) / 4;

    // offhand occupied means reduced block chance
    bool offhand_occupied = get_eq_char(ch, WEAR_SECONDARY) != NULL || get_eq_char(ch, WEAR_HOLD) != NULL;
    OBJ_DATA *wield = get_eq_char(ch, WEAR_WIELD);

    if ( wield != NULL && IS_WEAPON_STAT(wield, WEAPON_TWO_HANDS) )
        offhand_occupied = TRUE;

    if ( offhand_occupied )
        chance = chance * (100 + get_skill(ch, gsn_wrist_shield)) / 300;
    
    if ( ch->stance == STANCE_SWAYDES_MERCY || ch->stance == STANCE_AVERSION )
        chance += 10;

    if ( IS_AFFECTED(ch, AFF_SORE) )
        chance -= 10;

    if ( improve )
    {
        check_improve(ch, gsn_shield_block, TRUE, 15);
        if ( offhand_occupied )
            check_improve(ch, gsn_wrist_shield, TRUE, 20);
    }

    chance += mastery_bonus(ch, gsn_shield_block, 3, 5);

    return URANGE(0, chance, 75);
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

    chance = shield_block_chance(victim, TRUE);

    /* whips are harder to block */
    if ( get_weapon_sn(ch) == gsn_whip )
	chance -= 10;
    
    if ( !can_see_combat(victim,ch) && blind_penalty(victim) )
        chance -= chance / 4;

    if ( !per_chance(chance) )
        return FALSE;
    
    act_gag( "You block $n's attack with your shield.",  ch, NULL, victim, 
        TO_VICT, GAG_MISS );
    act_gag( "$N blocks your attack with a shield.", ch, NULL, victim, 
        TO_CHAR, GAG_MISS );
    act_gag( "$N blocks $n's attack with $S shield.", ch, NULL, victim, 
        TO_NOTVICT, GAG_MISS );
    return TRUE;
}

/*
 * Check for shield affect
 */
bool check_shield( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !IS_AFFECTED(victim, AFF_SHIELD) )
        return FALSE;
    
    int chance = 6;
        
    // whips are harder to block
    if ( get_weapon_sn(ch) == gsn_whip )
        chance /= 2;

    if ( !per_chance(chance) )
        return FALSE;
    
    act_gag( "Your shield deflects $n's attack.",  ch, NULL, victim, TO_VICT, GAG_MISS );
    act_gag( "$N's shield deflects your attack.", ch, NULL, victim, TO_CHAR, GAG_MISS );
    act_gag( "$N's shield deflects $n's attack.", ch, NULL, victim, TO_NOTVICT, GAG_MISS );

    return TRUE;
}

int dodge_chance( CHAR_DATA *ch, CHAR_DATA *opp, bool improve )
{
    int skill = get_skill(ch, gsn_dodge);

    if ( improve )
        check_improve( ch, gsn_dodge, TRUE, 15);

    if ( get_eq_char(ch, WEAR_WIELD) == NULL
         && get_eq_char(ch, WEAR_SHIELD) == NULL
         && get_eq_char(ch, WEAR_HOLD) == NULL )
    {
        skill += get_skill(ch, gsn_evasive);
        if (improve)
            check_improve(ch, gsn_evasive, TRUE, 15);
    }
    
    int opponent_adjust = 0;
    if ( opp )
    {
        int level_diff = ch->level - opp->level;
        int stat_diff = get_curr_stat(ch, STAT_AGI) - get_curr_stat(opp, STAT_DEX);
        opponent_adjust = (level_diff + stat_diff/4) / 2;
    }
    int chance = 10 + (skill + opponent_adjust) / 4;
    
    if ( ch->stance==STANCE_TOAD
        || ch->stance==STANCE_SWAYDES_MERCY
        || ch->stance==STANCE_AVERSION
        || ch->stance==STANCE_BUNNY
        || IS_SET(ch->form, FORM_DOUBLE_JOINTED) )
        chance += 15;

    if ( IS_AFFECTED(ch, AFF_SORE) )
        chance -= 10;
    
    chance += mastery_bonus(ch, gsn_dodge, 3, 5);
    
    return URANGE(0, chance, 75);
}

/*
* Check for dodge.
*/
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;
    
    if ( !IS_AWAKE(victim) )
        return FALSE;
    
    chance = dodge_chance(victim, ch, TRUE);

    if ( !can_see_combat(victim,ch) && blind_penalty(victim) )
        chance -= chance / 4;
    
    if ( !per_chance(chance) )
        return FALSE;

    act_gag( "You dodge $n's attack.", ch, NULL, victim, TO_VICT, GAG_MISS );
    act_gag( "$N dodges your attack.", ch, NULL, victim, TO_CHAR, GAG_MISS );
    act_gag( "$N dodges $n's attack.", ch, NULL, victim, TO_NOTVICT, GAG_MISS );

    return TRUE;
}




/*
 * Change a victim's position (during combat)
 */
void set_pos( CHAR_DATA *ch, int position )
{
    if ( ch->position == position )
        return;

    if ( ch->fighting )
        position = UMIN(position, POS_FIGHTING);

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

bool start_combat( CHAR_DATA *ch, CHAR_DATA *victim )
{
    attack_affect_strip(ch, victim);
    if ( !ch->fighting )
    {
        set_fighting(ch, victim);
        // double check that set_fighting worked in case kill_trigger stopped it
        if ( ch->fighting != victim )
            return FALSE;
    }
    if ( !victim->fighting )
        set_fighting_new(victim, ch, FALSE);
    return TRUE;
}

/*
 * check for kill trigger - returns wether attack was canceled
 */
bool check_kill_trigger( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *old_victim = ch->fighting;

    ch->fighting = victim;
    if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
	mp_percent_trigger( victim, ch, NULL,0, NULL,0, TRIG_KILL );
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
        
        if ( killer && !IS_NPC(killer) )
        {
            corpse->owner = str_dup(killer->name);
            eqloot = ( IS_SET(victim->act, PLR_HARDCORE) && IS_SET(killer->act, PLR_HARDCORE) )
                || ( IS_SET(victim->act, PLR_RP) && IS_SET(killer->act, PLR_RP) );
        }
        else
        {
            corpse->owner = str_dup(victim->name);
            eqloot = FALSE;
        }

        if (victim->gold > 1 || victim->silver > 1)
        {
            obj_to_obj(create_money(victim->gold / 2, victim->silver/2), corpse);
            victim->gold -= victim->gold/2;
            victim->silver -= victim->silver/2;
        }
        
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
        obj_next = obj->next_content;
        
        if (IS_SET(obj->extra_flags, ITEM_STICKY))
        {
            if (IS_NPC(victim))
            {
                if (obj->owner == NULL && killer)
                {
                    if ( IS_NPC(killer) && killer->master )
                        obj->owner = str_dup(killer->master->name);
                    else
                        obj->owner = str_dup(killer->name);
                }
            }
            else
                continue;
        }
        
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
        if (!IS_SET(ch->form, FORM_CONSTRUCT) && !IS_SET(ch->form, FORM_INTANGIBLE))
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
    curr_level_exp = exp_per_level(ch) * ch->level;
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

float calculate_exp_factor( CHAR_DATA *gch );
int calculate_base_exp( int power, CHAR_DATA *victim );
float get_vulnerability( CHAR_DATA *victim );
void adjust_alignment( CHAR_DATA *gch, CHAR_DATA *victim, int base_xp, float gain_factor );

void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    MEM_DATA *m;
    CHAR_DATA *gch, *leader;
    int members;
    int power, max_power, min_power, base_exp, min_base_exp, xp;
    int high_align, low_align;
    float group_factor, leadership, ch_factor;
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
    high_align = -1000;
    low_align = 1000;
    max_power = 1;
    min_power = 130;

    total_dam=0;
    group_dam=0;

    for (m = victim->aggressors; m; m=m->next)
        total_dam += m->reaction;
    total_dam = UMAX(1, total_dam);
    // damage is at least victim's max hitpoints - anything less is a bug or an exploit
    // e.g. having a charmie attack while not in the room (not remembered) to power-level low-level char
    total_dam = UMAX(victim->max_hit, total_dam);

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( IS_NPC(gch) || !is_same_group( gch, ch ) )
            continue;

        members++;

        power = level_power( gch );
        max_power = UMAX(max_power, power);
        min_power = UMIN(min_power, power);

        high_align = UMAX(high_align, gch->alignment);
        low_align = UMIN(low_align, gch->alignment);

        for (m=victim->aggressors; m; m=m->next)
            if (gch->id == m->id)
            {
                group_dam += m->reaction;
            }
    }
        
    // NPC killing NPC without being in group. Happens e.g. when charm wears off.
    if ( members == 0 )
        return;        

    min_base_exp = calculate_base_exp( max_power, victim );

    // group penalty for large group, high/low align and level range
    leader = ch->leader ? ch->leader : ch;
    leadership = (get_curr_stat(leader,STAT_CHA) + get_skill(leader, gsn_leadership)) / 300.0;
    group_factor = 1 - (high_align - low_align) / 4000.0 * (1 - leadership);
    group_factor *= 1 - (max_power - min_power) / 200.0 * (1 - leadership);
    group_factor *= 1.0/3 + 2.0/(3*members);

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	int dam_done;
        
        if ( IS_NPC(gch) || !is_same_group(gch, ch) )
            continue;
        
        base_exp = calculate_base_exp( level_power(gch), victim );
        dam_done = get_reaction( victim, gch );
        ch_factor = calculate_exp_factor( gch );
        
        // alignment change
        if ( dam_done > 0 && !IS_SET(victim->act,ACT_NOALIGN) && victim->pIndexData->vnum != gch->pcdata->questmob )
            adjust_alignment( gch, victim, base_exp, ((float) dam_done)/total_dam );
        
        // partly exp from own, partly from group
        xp = (min_base_exp * group_dam + (base_exp - min_base_exp) * dam_done) / total_dam;
        xp = number_range( xp * 9/10, xp * 11/10 );
        xp *= group_factor * ch_factor;

/* Removed since we are allowing certain people to play from same IP
 *	-Vodur 12/11/2011 
	  if ( ch != gch && is_same_player(ch, gch) )
	{
	    sprintf( buf, "Multiplay: %s gaining experience from %s's kill",
		     gch->name, ch->name );
	    cheat_log( buf );
	    wiznet(buf, ch, NULL, WIZ_CHEAT, 0, LEVEL_IMMORTAL);
	}
	else*/
	    gain_exp( gch, xp );
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
	pow = ch->level;
        // remort adjustment
        pow += ch->pcdata->remorts * (ch->level + 30) / 60;
	if ( ch->level > 90 )
	    pow += (ch->level - 90);
	return pow;
    }
}

// compute baseline xp for character of given level_power killing victim
int calculate_base_exp( int power, CHAR_DATA *victim )
{
    float base_exp, vuln;
    int base_value, mob_value, stance, stance_bonus, off_bonus;

    /* safety net */
    if ( !IS_NPC(victim) || IS_SET(victim->act, ACT_NOEXP) )
        return 0;

    // general base bonus/penalty
    base_exp = 100 + (victim->level - power);
    
    // adjust based on hp & damage dealt by mob compared to average mob at character's level

    // hitpoints - penalty or bonus
    base_value = level_base_hp(power);
    mob_value = mob_base_hp(victim->pIndexData, victim->level);
    if (mob_value <= base_value)
        base_exp += base_exp * (mob_value - base_value) / base_value;
    else
        base_exp += base_exp * (mob_value - base_value) / base_value * 3/4;
    
    // damage - penalty or bonus
    base_value = level_base_damage(power);
    mob_value = mob_base_damage(victim->pIndexData, victim->level);
    if (mob_value <= base_value)
        base_exp += base_exp * (mob_value - base_value) / base_value * 3/4;
    else
        base_exp += base_exp * (mob_value - base_value) / base_value * 2/3;
    
    // number of attacks - penalty or bonus
    base_value = level_base_attacks(power);
    mob_value = mob_base_attacks(victim->pIndexData, victim->level);
    if (mob_value <= base_value)
        base_exp += base_exp * (mob_value - base_value) / base_value * 2/3;
    else
        base_exp += base_exp * (mob_value - base_value) / base_value * 1/2;    

    // mobs with unusual hitroll/ac/saves
    mob_value = victim->pIndexData->hitroll_percent;
    if (mob_value < 100)
        base_exp += base_exp * (mob_value - 100) / 300;
    else
        base_exp += base_exp * UMIN(100, mob_value - 100) / 500;
    
    mob_value = victim->pIndexData->ac_percent;
    if (mob_value < 100)
        base_exp += base_exp * (mob_value - 100) / 300;
    else
        base_exp += base_exp * UMIN(100, mob_value - 100) / 500;
    
    mob_value = victim->pIndexData->saves_percent;
    if (mob_value < 100)
        base_exp += base_exp * (mob_value - 100) / 1000;
    else
        base_exp += base_exp * UMIN(100, mob_value - 100) / 2000;

    // adjustments for non-level dependent things
    
    /* reduce xp for mobs with lots of vulnerabilities */
    vuln = get_vulnerability( victim );
    base_exp -= base_exp * vuln * (2 - vuln) / 4;

    /* reward for tough mobs */
    if ( IS_SET(victim->res_flags, RES_WEAPON) || IS_SET(victim->imm_flags, IMM_WEAPON) )
        base_exp += base_exp/20;
    if ( IS_SET(victim->res_flags, RES_MAGIC) || IS_SET(victim->imm_flags, IMM_MAGIC) )
        base_exp += base_exp/20;
    if ( IS_SET(victim->pIndexData->affect_field, AFF_SANCTUARY) )
        base_exp += base_exp * 2/3;
    if ( IS_SET(victim->off_flags, OFF_FADE) )
        base_exp += base_exp/3;
    else if ( IS_SET(victim->pIndexData->affect_field, AFF_FADE) || IS_SET(victim->pIndexData->affect_field, AFF_CHAOS_FADE) )
        base_exp += base_exp/4;
    if ( IS_SET(victim->off_flags, OFF_DODGE) )
        base_exp += base_exp/10;
    if ( IS_SET(victim->off_flags, OFF_PARRY) )
        base_exp += base_exp/10;
    
    off_bonus = 0;
    off_bonus += IS_SET(victim->off_flags, OFF_AREA_ATTACK) ? 10 : 0;
    off_bonus += IS_SET(victim->off_flags, OFF_BASH) ? 5 : 0;
    off_bonus += IS_SET(victim->off_flags, OFF_DISARM) ? 5 : 0;
    off_bonus += IS_SET(victim->off_flags, OFF_KICK) ? 2 : 0;
    off_bonus += IS_SET(victim->off_flags, OFF_KICK_DIRT) ? 2 : 0;
    off_bonus += IS_SET(victim->off_flags, OFF_TAIL) ? 5 : 0;
    off_bonus += IS_SET(victim->off_flags, OFF_TRIP) ? 5 : 0;
    off_bonus += IS_SET(victim->off_flags, OFF_ARMED) ? 5 : 0;
    off_bonus += IS_SET(victim->off_flags, OFF_CIRCLE) ? 5 : 0;
    off_bonus += IS_SET(victim->off_flags, OFF_CRUSH) ? UMAX(0,3*victim->size - 5) : 0;
    base_exp += base_exp * off_bonus / 100;

    if (victim->pIndexData->spec_fun != NULL)
    {
        SPEC_FUN *spec = victim->pIndexData->spec_fun;
        // dragons
        if ( spec == spec_breath_any
            || spec == spec_breath_acid
            || spec == spec_breath_fire
            || spec == spec_breath_frost
            || spec == spec_breath_gas
            || spec == spec_breath_lightning )
            base_exp += base_exp / 3;
        // casters
        else if ( spec == spec_cast_cleric
            || spec == spec_cast_mage
            || spec == spec_cast_undead )
            base_exp += base_exp / 2;
        // other
        else if ( spec == spec_thief
            || spec == spec_nasty )
            base_exp += base_exp / 10;
    }

    if ( IS_SET(victim->pIndexData->act, ACT_AGGRESSIVE) )
        base_exp += base_exp/10;

    stance = victim->pIndexData->stance;
    if ( stance != STANCE_DEFAULT )
    {
        // adjust stance cost for purpose of bonus calculation
        switch (stance) {
            case STANCE_BLOODBATH:
                stance_bonus = 10;
                break;
            case STANCE_TORTOISE:
                stance_bonus = 15;
                break;
            case STANCE_SHADOWWALK:
                stance_bonus = 20;
                break;
            default:
                stance_bonus = stances[stance].cost;
                break;
        }
        base_exp += base_exp * stance_bonus / 60;
    }

    // reduce extreme amounts of base xp
    if (base_exp > 100)
        base_exp = sqrt(base_exp) * 20 - 100;
    
    return (int)base_exp;
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer ( by gain_align percentage )
 * Edit this function to change xp computations.
 */
float calculate_exp_factor( CHAR_DATA *gch )
{
    float xp_factor = 1;
    int bonus = 0;

    /* safety net */
    if ( IS_NPC(gch) )
	return 0;

    // penalty for high-level chars - levels above 90 provide bonus-dice, so harder to achieve
    if ( gch->level >= 90 )
    {
        int hitdice_gained = gch->level - 88;
        xp_factor *= 5.0 / (4 + hitdice_gained);
    }
    // and bonus for low-ish characters
    else
        xp_factor *= (300 - gch->level) / 200.0;
    // bonus for newbies
    if ( gch->pcdata->remorts == 0 )
        xp_factor *= (300 - gch->level) / 200.0;
    // bonus for first 5 levels
    if ( gch->level <= 5 )
        xp_factor *= 1.5;

    // additive bonuses
    
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

    xp_factor *=  1 + bonus / 100.0;

    return xp_factor;
}

// returns "degree of of vulnerability" (0-1) - for xp calculation
float get_vulnerability( CHAR_DATA *victim )
{
    int vuln = 0;
    if (IS_SET(victim->vuln_flags, VULN_WEAPON))
        vuln += 27;
    else
    {
        if (IS_SET(victim->vuln_flags, VULN_BASH))
            vuln += 9;
        if (IS_SET(victim->vuln_flags, VULN_PIERCE))
            vuln += 9;
        if (IS_SET(victim->vuln_flags, VULN_SLASH))
            vuln += 9;        
    }
    if (IS_SET(victim->vuln_flags, VULN_MAGIC))
        vuln += 23;
    else
    {
        if (IS_SET(victim->vuln_flags, VULN_FIRE))
            vuln += 4;
        if (IS_SET(victim->vuln_flags, VULN_COLD))
            vuln += 3;
        if (IS_SET(victim->vuln_flags, VULN_LIGHTNING))
            vuln += 3;
        if (IS_SET(victim->vuln_flags, VULN_ACID))
            vuln += 2;
        if (IS_SET(victim->vuln_flags, VULN_POISON))
            vuln += 2;
        if (IS_SET(victim->vuln_flags, VULN_NEGATIVE))
            vuln += 1;
        if (IS_SET(victim->vuln_flags, VULN_HOLY))
            vuln += 1;
        if (IS_SET(victim->vuln_flags, VULN_ENERGY))
            vuln += 1;
        if (IS_SET(victim->vuln_flags, VULN_MENTAL))
            vuln += 1;
        if (IS_SET(victim->vuln_flags, VULN_DISEASE))
            vuln += 2;
        if (IS_SET(victim->vuln_flags, VULN_DROWNING))
            vuln += 1;
        if (IS_SET(victim->vuln_flags, VULN_LIGHT))
            vuln += 1;
        if (IS_SET(victim->vuln_flags, VULN_SOUND))
            vuln += 1;
    }
    
    return vuln / 50.0;
}

void adjust_alignment( CHAR_DATA *gch, CHAR_DATA *victim, int base_xp, float gain_factor )
{
    int galign = gch->alignment;
    int valign = victim->alignment;
    int change;

    // killing neutral-aligned non-sentients or undeads does nothing; same for aggro mobs
    if ( IS_NEUTRAL(victim) && (!IS_SET(victim->form, FORM_SENTIENT) || IS_UNDEAD(victim) || NPC_ACT(victim, ACT_AGGRESSIVE)) )
        return;
    
    // killing evil victims makes you good, killing neutral or good victims makes you evil
    change = - (350 + victim->alignment);
    // adjust based on xp gained - redemption requires effort, but killing harmless victims is still pretty evil
    if (change > 0)
        change = change * UMAX(25, base_xp) / 100;
    else
        change = change * UMAX(50, base_xp) / 100;
    // scaling
    change = change / 50;
        
    change_align(gch, change * gain_factor);
    return;
}        

/* return gagtype if any */
int get_damage_messages( int dam, int dt, char **vs, char **vp, char *punct )
{
    int gag_type=0;
    if (dam<100)
        if (dam < 39)
        {
            if ( dam < 1 )
            { 
                *vs = "miss"; *vp = "misses";
                if ( is_normal_hit(dt) || dt == gsn_bite || dt == gsn_chop || dt == gsn_kick )
                    gag_type = GAG_MISS;
            }
            else if ( dam <   2 ) { *vs = "{mbother{ ";  *vp = "{mbothers{ "; }
            else if ( dam <   5 ) { *vs = "{mscratch{ "; *vp = "{mscratches{ ";   }
            else if ( dam <   8 ) { *vs = "{mbruise{ ";  *vp = "{mbruises{ "; }
            else if ( dam <  11 ) { *vs = "{bglance{ ";  *vp = "{bglances{ "; }
            else if ( dam <  15 ) { *vs = "{bhurt{ ";    *vp = "{bhurts{ ";   }
            else if ( dam <  20 ) { *vs = "{Bgraze{ ";   *vp = "{Bgrazes{ ";  }
            else if ( dam <  27 ) { *vs = "{Bhit{ ";     *vp = "{Bhits{ ";    }
            else if ( dam <  31 ) { *vs = "{Binjure{ ";  *vp = "{Binjures{ "; }
            else if ( dam <  35 ) { *vs = "{Cwound{ ";   *vp = "{Cwounds{ ";  }
            else { *vs = "{CPUMMEL{ ";  *vp = "{CPUMMELS{ "; }
        } else {
            if ( dam <  44 ) { *vs = "{CMAUL{ ";        *vp = "{CMAULS{ ";   }
            else if ( dam <  48 ) { *vs = "{GDECIMATE{ ";    *vp = "{GDECIMATES{ ";   }
            else if ( dam <  52 ) { *vs = "{GDEVASTATE{ ";   *vp = "{GDEVASTATES{ ";}
            else if ( dam <  56 ) { *vs = "{GMAIM{ ";        *vp = "{GMAIMS{ ";   }
            else if ( dam <  60 ) { *vs = "{yMANGLE{ ";  *vp = "{yMANGLES{ "; }
            else if ( dam <  64 ) { *vs = "{yDEMOLISH{ ";    *vp = "{yDEMOLISHES{ ";}
            else if ( dam <  70 ) { *vs = "*** {yMUTILATE{  ***"; *vp = "*** {yMUTILATES{  ***";}
            else if ( dam <  80 ) { *vs = "*** {YPULVERIZE{  ***";   *vp = "*** {YPULVERIZES{  ***";}
            else if ( dam <  90 ) { *vs = "=== {YDISMEMBER{  ==="; *vp = "=== {YDISMEMBERS{  ===";}
            else { *vs = "=== {YDISEMBOWEL{  ==="; *vp = "=== {YDISEMBOWELS{  ===";}
        }
        else
            if (dam < 220)
            {
                if ( dam <  110) { *vs = ">>> {rMASSACRE{  <<<"; *vp = ">>> {rMASSACRES{  <<<";}
                else if ( dam < 120)  { *vs = ">>> {rOBLITERATE{  <<<"; *vp = ">>> {rOBLITERATES{  <<<";}
                else if ( dam < 135)  { *vs = "{r<<< ANNIHILATE >>>{ "; *vp = "{r<<< ANNIHILATES >>>{ ";}
                else if ( dam < 150)  { *vs = "{r<<< DESTROY >>>{ "; *vp = "{r<<< DESTROYS >>>{ ";}
                else if ( dam < 165)  { *vs = "{R!!! ERADICATE !!!{ "; *vp = "{R!!! ERADICATES !!!{ ";}
                else if ( dam < 190)  { *vs = "{R!!! LIQUIDATE !!!{ "; *vp = "{R!!! LIQUIDATES !!!{ ";}
                else { *vs = "{RXXX VAPORIZE XXX{ "; *vp = "{RXXX VAPORIZES XXX{ ";}
            } else {
                if ( dam < 250)  { *vs = "{RXXX DISINTEGRATE XXX{ "; *vp = "{RXXX DISINTEGRATES XXX{ ";}
                else if ( dam < 300)  { *vs = "do {+SICKENING{  damage to"; *vp = "does {+SICKENING{  damage to";}
                else if ( dam < 400)  { *vs = "do {+INSANE{  damage to"; *vp = "does {+INSANE{  damage to";}
                else if ( dam < 600)  { *vs = "do {+UNSPEAKABLE{  things to"; *vp = "does {+UNSPEAKABLE{  things to";}
                else if ( dam < 1000)  { *vs = "do {+{%BLASPHEMOUS{  things to"; *vp = "does {+{%BLASPHEMOUS{  things to";}
                else if ( dam < 1500)  { *vs = "do {+{%UNBELIEVABLE{  things to"; *vp = "does {+{%UNBELIEVABLE{  things to";}
                else    { *vs = "do {+{%INCONCEIVABLE{  things to"; *vp = "does {+{%INCONCEIVABLE{  things to";}
            }

    *punct   = (dam < 31) ? '.' : '!';

    return gag_type;

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
    
#ifdef DEBUG_DAMTYPE
    if (dt == 1000 + DEBUG_DAMTYPE) {
	logpf("critical damage(A): %s dealing full %d damage to %s at %d", ch->name, dam, victim->name, ch->in_room->vnum);
    }
#endif
    if (ch == NULL || victim == NULL)
        return;

	sprintf(buf, " for %d damage", dam);
	#ifdef TESTER
	chmeter = buf;
	victmeter = buf;
	#else
	chmeter = ((IS_AFFECTED(ch, AFF_BATTLE_METER) && (dam>0)) ? buf : "");
	victmeter = ((IS_AFFECTED(victim, AFF_BATTLE_METER) && (dam>0)) ? buf : "");
	#endif

    gag_type = get_damage_messages( dam, dt, &vs, &vp, &punct);
    
#ifdef DEBUG_DAMTYPE
    if (dt == 1000 + DEBUG_DAMTYPE) {
	logpf("critical damage(B): %s dealing full %d damage to %s at %d", ch->name, dam, victim->name, ch->in_room->vnum);
    }
#endif
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
    
#ifdef DEBUG_DAMTYPE
    if (dt == 1000 + DEBUG_DAMTYPE) {
	logpf("critical damage(C): %s dealing full %d damage to %s at %d", ch->name, dam, victim->name, ch->in_room->vnum);
    }
#endif
    
    if ( immune )
	gag_type = GAG_IMMUNE;

    if ( dt < MAX_SKILL && skill_table[dt].pgsn != NULL )    
    {
        sn = *skill_table[dt].pgsn;
        if ( sn == gsn_electrocution
            || sn == gsn_immolation
            || sn == gsn_absolute_zero
            || sn == gsn_epidemic
            || sn == gsn_quirkys_insanity
            || sn == gsn_phantasmal_image )
        gag_type = GAG_AURA;
    }

    if (ch == victim)
    {
        if (IS_SET (ch->form, FORM_SUNBURN) && dt == gsn_torch)
            gag_type = GAG_SUNBURN;

        act_gag(buf1,ch,NULL,NULL,TO_ROOM, gag_type);
        act_gag(buf2,ch,NULL,NULL,TO_CHAR, gag_type);
    }
    else
    {
        act_gag( buf1, ch, NULL, victim, TO_NOTVICT, gag_type);
        act_gag( buf2, ch, NULL, victim, TO_CHAR, gag_type);
        act_gag( buf3, ch, NULL, victim, TO_VICT, gag_type);
    }
    
#ifdef DEBUG_DAMTYPE
    if (dt == 1000 + DEBUG_DAMTYPE) {
	logpf("critical damage(D): %s dealing full %d damage to %s at %d", ch->name, dam, victim->name, ch->in_room->vnum);
    }
#endif

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

int direction_lookup( char *arg1 )
{
    if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) return DIR_NORTH;
    if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))  return DIR_EAST;
    if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) return DIR_SOUTH;
    if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))  return DIR_WEST;
    if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" ))   return DIR_UP;
    if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))  return DIR_DOWN;
    if (!str_cmp(arg1, "ne") || !str_cmp(arg1, "northeast")) return DIR_NORTHEAST;
    if (!str_cmp(arg1, "se") || !str_cmp(arg1, "southeast")) return DIR_SOUTHEAST;
    if (!str_cmp(arg1, "sw") || !str_cmp(arg1, "southwest")) return DIR_SOUTHWEST;
    if (!str_cmp(arg1, "nw") || !str_cmp(arg1, "northwest")) return DIR_NORTHWEST;

    return -1;
    
}

int get_exit_count( CHAR_DATA *ch )
{
    int d, count = 0;
    for ( d = 0; d < MAX_DIR; d++ )
        if ( can_move_dir(ch, d, FALSE) )
            count++;
    return count;
}

void do_flee( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[80];
    ROOM_INDEX_DATA *was_in, *now_in;
    CHAR_DATA *opp;
    int dir, exit_count;
    
    if ( IS_AFFECTED(ch, AFF_FLEE) )
       return;

    if ( !ch->fighting )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }
    
    one_argument( argument, arg );

    if ( (was_in = ch->in_room) == NULL )
        return;

    if ( arg[0] )
    {
        if ( (dir = direction_lookup(arg)) == -1 )
        {
            send_to_char("That isn't a direction!\n\r", ch);
            return;
        }
        if ( !can_move_dir(ch, dir, TRUE) )
            return;
        exit_count = 1;
    }
    else
    {
        if ( (dir = get_random_exit(ch)) == -1 )
        {
            send_to_char("There is nowhere to run!\n\r", ch);
            return;
        }
        exit_count = get_exit_count(ch);
    }
    
    // we now have a chance to escape, so lag is given now, regardless of success
    int wait = rand_div(PULSE_VIOLENCE * (10 - mastery_bonus(ch, gsn_flee, 4, 5)), 10);
    if ( ch->stance == STANCE_BUNNY )
        wait = rand_div(wait, 2);
    WAIT_STATE(ch, wait);
    
    // auto-fail chance based on number of available exits
    if ( number_bits(exit_count) == 0 )
    {
        // retreat skill allows fleeing regardless of number of exits
        if ( per_chance(get_skill(ch, gsn_retreat))  )
            check_improve(ch, gsn_retreat, TRUE, 4);
        else
        {
            check_improve(ch, gsn_retreat, FALSE, 4);
            send_to_char("PANIC! You couldn't escape!\n\r", ch);
            return;
        }
    }

    if ( IS_AFFECTED(ch, AFF_ENTANGLE) && !per_chance(get_curr_stat(ch, STAT_LUC) / 10) )
    {
        send_to_char("The plants entangling you hold you in place!\n\r", ch);
        return;
    }
    
    if ( is_affected(ch, gsn_net) )
    {
        /* Chance of breaking the net:  str/15 < 13.5% */
        if ( per_chance(get_curr_stat(ch, STAT_STR) / 15) )
        {
            send_to_char( "You rip the net apart!\n\r", ch );
            act("$n rips the net apart!", ch, NULL, NULL, TO_ROOM);
            affect_strip( ch, gsn_net );
        }
        /* Chance of struggling out of the net:  agi/15 < 13.5% */
        else if ( per_chance(get_curr_stat(ch, STAT_AGI) / 15) )
        {
            send_to_char( "You struggle free from the net!\n\r", ch );
            act("$n struggles free from the net!", ch, NULL, NULL, TO_ROOM);
            affect_strip( ch, gsn_net );
        }
        /* Chance of fleeing while still trapped in the net:  luc/15 < 13.5% */
        else if ( !per_chance(get_curr_stat(ch, STAT_LUC) / 15) )
        {
            send_to_char( "You struggle in the net, and can't seem to get away.\n\r", ch );
            act("$n struggles in the net.", ch, NULL, NULL, TO_ROOM);
            return;
        }
    }
    
    int ch_base = (10 + ch->level) * (100 + get_skill(ch, gsn_flee)) / 100;
    int ch_roll = number_range(0, ch_base);

    if ( ch->slow_move > 0 )
    {
        int second_roll = number_range(0, ch_base);
        ch_roll = UMIN(ch_roll, second_roll);
    }
    
    for ( opp = ch->in_room->people; opp != NULL; opp = opp->next_in_room )
    {
        if ( opp->fighting != ch || is_wimpy(opp) )
            continue;
        
        // harder to flee from PCs
        int entrapment_factor = (IS_NPC(opp) ? 100 : 200) + get_skill(opp, gsn_entrapment);
        int opp_base = (10 + opp->level) * entrapment_factor / 100;
        int opp_roll = number_range(0, opp_base);

        //printf_to_char(ch, "ch_roll(%d) = %d vs %d = opp_roll(%d)\n\r", ch_base, ch_roll, opp_roll, opp_base);

        if ( opp_roll > ch_roll || opp->stance == STANCE_AMBUSH && number_bits(1) )
        {
            if ( per_chance(mastery_bonus(opp, gsn_entrapment, 60, 100)) )
            {
                act("$N trips you over, ending your escape attempt!", ch, NULL, opp, TO_CHAR);
                act("You trip $n over, ending $s escape attempt!", ch, NULL, opp, TO_VICT);
                act("$N trips $n over, ending $s escape attempt!", ch, NULL, opp, TO_NOTVICT);
                set_pos(ch, POS_RESTING);
                check_lose_stance(ch);
                // free attack
                one_hit(opp, ch, TYPE_UNDEFINED, FALSE);
                if ( per_chance(offhand_attack_chance(opp, TRUE)) )
                    one_hit(opp, ch, TYPE_UNDEFINED, TRUE);
            }
            else
            {
                act("$N jumps in your way, blocking your escape!", ch, NULL, opp, TO_CHAR);
                act("You jump in $n's way, blocking $s escape!", ch, NULL, opp, TO_VICT);
                act("$N jumps in $n's way, blocking $s escape!", ch, NULL, opp, TO_NOTVICT);
            }
            check_improve(opp, gsn_entrapment, TRUE, 1);
            check_improve(ch, gsn_flee, FALSE, 4);
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
    now_in = ch->in_room;
    
    if ( now_in == was_in )
    {
        send_to_char( "You get turned around and flee back into the room!\n\r", ch );
        return;
    }

    check_improve(ch, gsn_flee, TRUE, 4);

    /* char might have been transed by an mprog */
    if ( dir == -1 )
        sprintf(buf, "$n has fled!");
    else
        sprintf(buf, "$n has fled %s!", dir_name[dir]);
        
    ch->in_room = was_in;
    act(buf, ch, NULL, NULL, TO_ROOM);
    ch->in_room = now_in;
        
    if ( dir == -1 )
        printf_to_char(ch, "You flee from combat!\n\r");
    else
        printf_to_char(ch, "You flee %s from combat!\n\r", dir_name[dir]);

    if ( !IS_NPC(ch) && !IS_HERO(ch) && !IS_SET(ch->act, PLR_WAR) )
    {
        // Thieves are exempt from XP penalty
        if ( ch->class == 1 )
            send_to_char("You snuck away safely.\n\r", ch);
        else
        {
            send_to_char("You lost 10 exp.\n\r", ch);
            gain_exp(ch, -10);
        }
    }

    if ( ch->pcdata && in_pkill_battle(ch) )
        ch->pcdata->pkill_timer = UMAX(ch->pcdata->pkill_timer, 10 * PULSE_VIOLENCE);
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

	    destance(victim, get_mastery(opp, gsn_hogtie));
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

	if ( opp->fighting != victim || !can_see_combat(opp, victim) )
	    continue;
	
	wield = get_eq_char( opp, WEAR_WIELD );
	/* ranged weapons get off one shot */
	if ( is_ranged_weapon(wield) )
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

  if ( IS_NPC(victim) || wants_to_rescue(victim) )
      return victim;
  
  for (ch = victim->in_room->people; ch != NULL; ch = ch->next_in_room)
  {
      if ( !wants_to_rescue(ch)
	   || !is_same_group(ch, victim)
	   || ch == victim || ch == attacker )
	  continue;
      if (is_safe_spell(attacker, ch, FALSE)
	  || ch->position <= POS_SLEEPING 
	  || !check_see_combat(ch, attacker))
	  continue;

      chance = 25 + get_skill(ch, gsn_bodyguard) / 2 - ass_skill / 4;
      chance += (ch->level - attacker->level) / 4;
      if ( per_chance(chance) )
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

// players may want to stop raging to preserve moves
/*
void do_calm( CHAR_DATA *ch, char *argument )
{
    if ( !IS_AFFECTED(ch, AFF_BERSERK) )
    {
        send_to_char("You are already calm.\n\r", ch);
        return;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);
    
    // may not succeed while fighting
    if ( ch->fighting != NULL )
    {
        int chance = 25 + get_curr_stat(ch, STAT_DIS) / 4;
        chance += 25 * ch->hit / UMAX(1, ch->max_hit);
        chance -= 25 * ch->move / UMAX(1, ch->max_move);
        
        if ( number_percent() > chance )
        {
            send_to_char("You fail to control your anger.\n\r", ch);
            return;            
        }
    }
    
    affect_strip_flag(ch, AFF_BERSERK);
    // safety-net just in case we fail - e.g. races with permanent berserk
    if ( IS_AFFECTED(ch, AFF_BERSERK) )
    {
        send_to_char("Your rage seems uncontrollable.\n\r", ch);
        return;
    }
    
    send_to_char("You control your anger and calm down.\n\r", ch);
    act("$n appears to calm down.", ch, NULL, NULL, TO_ROOM);
    
    return;
}
*/

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
    int sn = *(stances[stance].gsn);
    int skill = get_skill(ch, sn);
    int cost = stances[stance].cost * (140-skill)/40;

    cost -= cost * mastery_bonus(ch, sn, 20, 30) / 100;

    return cost;
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

    deduct_move_cost(ch, cost);

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
        if (!ch->stance && stances[ch->stance].name)
        {
            char buffer[80];
            if (sprintf(buffer, "You do not know that stance so you stay in the %s stance.\n\r", stances[i].name) > 0)
            {
                send_to_char(buffer, ch);    
                return;
            }
        }
        
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
bool check_lose_stance( CHAR_DATA *ch )
{
    if ( ch->stance == 0 )
        return FALSE;
    
    int sn = *(stances[ch->stance].gsn);
    int skill = get_skill(ch, sn);
    
    if ( per_chance(mastery_bonus(ch, sn, 20, 30)) )
        return FALSE;

    if ( per_chance(skill * 9/10) ) /* Always keep 10% chance of failure */
        return FALSE;

    send_to_char("You lose your stance!\n\r", ch);
    ch->stance = 0;
    return TRUE;
}

bool destance( CHAR_DATA *ch, int attack_mastery )
{
    if ( ch->stance == 0 )
        return FALSE;
    
    int sn = *(stances[ch->stance].gsn);
    int mastery = get_mastery(ch, sn) - attack_mastery;
    
    if ( per_chance( mastery == 1 ? 20 : mastery == 2 ? 30 : 0) )
        return FALSE;
    
    send_to_char("You lose your stance!\n\r", ch);
    ch->stance = 0;
    return TRUE;
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

#ifdef TESTER
void do_fstat( CHAR_DATA *ch, char *argument)
{
    if ( argument[0] != '\0' && !str_prefix( argument, "clear" ) )
    {
	ch->attacks_success=0;
        ch->attacks_misses=0;
        ch->damage_dealt=0;
        ch->damage_taken=0;
        ch->mana_used=0;
        ch->moves_used=0;
	send_to_char("Fstat cleared.\n\r",ch);
    }
	
	send_to_char("\n",ch);
	printf_to_char(ch, "%-30s %20d\n", "Hits:", ch->attacks_success);
	printf_to_char(ch, "%-30s %20d\n", "Misses:", ch->attacks_misses);
	printf_to_char(ch, "%-30s %20d\n", "Damage dealt:", ch->damage_dealt);
	printf_to_char(ch, "%-30s %20f\n", "Avg damage per hit:", (float)ch->damage_dealt/ch->attacks_success ); 
	printf_to_char(ch, "%-30s %20f\n", "Avg damage per attempt:", (float)ch->damage_dealt/(ch->attacks_success + ch->attacks_misses) );

	send_to_char("\n",ch);
	printf_to_char(ch, "%-30s %20d\n", "Damage taken:", ch->damage_taken);
	printf_to_char(ch, "%-30s %20d\n", "Mana used:", ch->mana_used);
	printf_to_char(ch, "%-30s %20d\n", "Moves used:", ch->moves_used);
}
	
#endif
