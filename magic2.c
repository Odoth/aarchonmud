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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "tables.h"
#include "religion.h"

bool remove_obj  args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void wear_obj    args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
void do_flee     args( ( CHAR_DATA *ch, char *argument ) );
bool check_spell_disabled args( (const struct skill_type *command) );

bool saves_dispel args(( int dis_level, int spell_level, int duration));
bool check_dispel args(( int dis_level, CHAR_DATA *victim, int sn));

void dam_message  args(( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, bool immune ));
int  hit_gain     args(( CHAR_DATA *ch ));
int  mana_gain    args(( CHAR_DATA *ch ));
int  move_gain    args(( CHAR_DATA *ch )) ;
bool check_dispel( int dis_level, CHAR_DATA *victim, int sn);

RELIGION_DATA *get_religion args(( CHAR_DATA *ch ));

ROOM_INDEX_DATA* get_portal_room( char *name );

DECLARE_DO_FUN(do_scan      );
DECLARE_DO_FUN(do_look      );
DECLARE_DO_FUN(do_where );

extern char *target_name;

void spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    do_where(ch, target_name);
}

void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    ROOM_INDEX_DATA *room;
    OBJ_DATA *portal, *stone;

    if ( !strcmp(target_name, "locate") )
    {
	show_portal_names( ch );
	return;
    }

    if ( !can_cast_transport(ch) )
	return;

    if ( target_name[0] == '\0' )
    {
	send_to_char( "Where should the portal lead to?\n\r", ch );
	return;
    }

    if ( (room = get_portal_room(target_name)) == NULL
	 || !can_see_room(ch, room)
	 || !is_room_ingame(room) )
    {
	send_to_char( "Spell failed to create a portal.\n\r", ch );
	return;
    }

    stone = get_eq_char(ch,WEAR_HOLD);

    /*
    if ( stone == NULL || stone->item_type != ITEM_WARP_STONE )
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }
    */
    
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->value[3] = room->vnum;
    
    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
        extract_obj(stone);
	portal->timer = 10 + level * 2;
    }
    else
    {
	portal->timer = 2;
	I_SET_BIT( portal->value[2], GATE_BUGGY );
    }
    
    obj_to_room(portal,ch->in_room);
    
    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;
    
    if ( !strcmp(target_name, "locate") )
    {
	show_portal_names( ch );
	return;
    }

    if ( !can_cast_transport(ch) )
	return;

    from_room = ch->in_room;

    if ( target_name[0] == '\0' )
    {
	send_to_char( "Where should the portal lead to?\n\r", ch );
	return;
    }

    if ( (to_room = get_portal_room(target_name)) == NULL
	 || !can_see_room(ch, to_room)
	 || !is_room_ingame(to_room) )
    {
	send_to_char( "Spell failed to create a nexus.\n\r", ch );
	return;
    }

    stone = get_eq_char(ch,WEAR_HOLD);
    if ( stone != NULL && stone->item_type != ITEM_WARP_STONE )
	stone = NULL;

    /*
    if ( stone == NULL || stone->item_type != ITEM_WARP_STONE )
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }
    */
    
    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
        extract_obj(stone);
    }
    
    /* portal one */ 
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->value[3] = to_room->vnum;

    if ( stone != NULL )
	portal->timer = 5 + level;
    else
    {
	portal->timer = 2;
	I_SET_BIT( portal->value[2], GATE_BUGGY );
    }
    
    obj_to_room(portal,from_room);
    
    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
    
    /* no second portal if rooms are the same */
    if (to_room == from_room)
        return;
    
    /* portal two */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->value[3] = from_room->vnum;
    
    if ( stone != NULL )
	portal->timer = 5 + level;
    else
    {
	portal->timer = 2;
	I_SET_BIT( portal->value[2], GATE_BUGGY );
    }

    obj_to_room(portal,to_room);
    
    if (to_room->people != NULL)
    {
        act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
        act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
    }
}

/* Gunslinger spells by Siva */
void spell_call_sidekick( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    CHAR_DATA *mob;
    CHAR_DATA *check;
    char buf[MAX_STRING_LENGTH];
    int mlevel, chance;
    
    send_to_char( "You call out to your noble sidekick!\n\r", ch );
    act( "$n calls out to $s noble sidekick.", ch, NULL, NULL, TO_ROOM );
    
    if (IS_SET(ch->act, PLR_WAR))
    {
        send_to_char("Sidekicks have no place in wars like these.\n\r", ch );
        return;
    }
    
    /* Check number of charmees against cha*/ 
    mlevel = URANGE(1, level / 2, ch->level);
    if ( check_cha_follow(ch, mlevel) < mlevel )
        return;
    
    chance = (get_curr_stat(ch, STAT_LUC)) / 2;
    
    if ( number_percent() > chance ) 
    {
        send_to_char( "Your sidekick doesn't answer your call.\n\r", ch);
        return;
    }
    
    if ((mob = create_mobile(get_mob_index(MOB_VNUM_SIDEKICK)))==NULL) 
        return;
    
    set_mob_level( mob, mlevel );
    arm_npc( mob );

    sprintf(buf,"%s\n\rThis sidekick faithfully follows %s.\n\r\n\r",
        mob->description,ch->name);
    free_string(mob->description);
    mob->description = str_dup(buf);
    
    char_to_room( mob, ch->in_room );
    
    send_to_char( "Your sidekick answers the call!\n\r", ch );
    act( "$n's sidekick is here to kick some ass!", ch, NULL, NULL, TO_ROOM );
    add_follower( mob, ch );
    mob->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level   = level;
    af.duration  = get_duration(sn, level);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );
    
    return;
}

/*
void spell_intimidation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
    
    if (!IS_AFFECTED(victim,AFF_SANCTUARY)) 
    {
        send_to_char("Your opponent isn't protected by Sanctuary.\n\r",ch);
        return;
    }
    
    if (saves_spell(level+10, victim, DAM_OTHER))
    {
        act( "$n tried to intimidate you, but you won't take $s crap.",ch,NULL,
            victim,TO_VICT);
        act( "You don't really intimidate $N.", ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level+10,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $N's body withers away.",
            ch,NULL,victim,TO_NOTVICT);
        found = TRUE;
    }
    
    if (found)
        act("You intimidate $N out of $S comfy sanctuary.",ch,NULL,victim,
        TO_CHAR);
    else
        send_to_char("Nope, that didn't quite work.\n\r",ch);
    return;
}

void spell_dowsing(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *spring, *stick;
    
    stick = get_eq_char(ch,WEAR_HOLD);
    if ((stick == NULL) || (stick->item_type != ITEM_DOWSING_STICK))
    {
        send_to_char("You'll need a dowsing stick for that.\n\r",ch);
        return;
    }
    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level * 2;
    obj_to_room( spring, ch->in_room );
    act( "$n has divined the location of a spring!", ch, spring, NULL, TO_ROOM );
    act( "You sense water and draw forth a spring.", ch, spring, NULL, TO_CHAR );
    return;
}

void spell_rustle_grub ( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *mushroom;
    
    mushroom = create_object( get_obj_index( OBJ_VNUM_GRUB ), 0 );
    mushroom->value[0] = level / 3;
    mushroom->value[1] = level;
    obj_to_room( mushroom, ch->in_room );
    act( "$n rustles up some grub.", ch, mushroom, NULL, TO_ROOM );
    act( "You rustle up some grub.", ch, mushroom, NULL, TO_CHAR );
    return;
}
*/

// everyone who fails a save makes an attack against the target
void spell_betray( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *traitor, *traitor_next;

    // ensure the victim attacks caster rather than traitor if used to initiate combat
    if ( !victim->fighting )
        start_combat(victim, ch);
    
    act("You project feelings of hate towards $N.", ch, NULL, victim, TO_CHAR);

    for ( traitor = victim->in_room->people; traitor != NULL; traitor = traitor_next )
    {
        traitor_next = traitor->next_in_room;
        
        if ( IS_DEAD(victim) || traitor->in_room != victim->in_room )
            return;

        if ( traitor == victim || traitor->fighting == victim || traitor->stop > 0 )
            continue;

        if ( is_safe_spell(ch, traitor, TRUE) || is_safe_spell(traitor, victim, FALSE) )
            continue;

        if ( saves_spell(traitor, ch, level, DAM_MENTAL) )
        {
            act("You feel a momentary urge to attack $N.", traitor, NULL, victim, TO_CHAR);
            act("$N resists your command.", ch, NULL, traitor, TO_CHAR);
            if ( !traitor->fighting )
                start_combat(traitor, ch);
            continue;
        }

        act("You are overcome with blind hate for $N!", traitor, NULL, victim, TO_CHAR);
        act("$n strikes at you with hate in $s eyes!", traitor, NULL, victim, TO_VICT);
        act("$n strikes at $N with hate in $s eyes!", traitor, NULL, victim, TO_NOTVICT);
        
        // one full attack against the target, the return to normal fight
        CHAR_DATA *was_fighting = traitor->fighting;
        set_fighting_new(traitor, victim, FALSE);
        multi_hit(traitor, victim, TYPE_UNDEFINED);
        set_fighting_new(traitor, was_fighting ? was_fighting : ch, FALSE);

        // this uses up the next full attack for the traitor
        traitor->stop += 1;
        traitor->wait += PULSE_VIOLENCE;
    }
    
    return;
}


void spell_astral( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_ASTRAL) )
        return;
    
    /*if (IS_REMORT(victim))
    {
        send_to_char("The Astral plane doesnt intersect with remort.\n\r",victim);
        return;
    }
    */

    if (IS_NOHIDE(ch))
    {
        send_to_char("The astral plane cannot be reached from here.\n\r",ch);
        return;
    }

    if (IS_TAG(ch))
    {
        send_to_char("There is no place to hide in freeze tag.\n\r", ch );
        return;
    }
    
    act( "$n steps into the Astral plane.", victim, NULL, NULL, TO_ROOM );
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ASTRAL;
    affect_to_char( victim, &af );
    affect_strip ( ch, gsn_hide );
    REMOVE_BIT   ( ch->affect_field, AFF_HIDE   );
    affect_strip ( ch, gsn_invis );
    REMOVE_BIT   ( ch->affect_field, AFF_INVISIBLE  );
    affect_strip ( ch, gsn_sneak );
    REMOVE_BIT   ( ch->affect_field, AFF_SNEAK  );
    send_to_char( "You step into the Astral plane.\n\r", victim );
    return;
}

void spell_detect_astral( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_DETECT_ASTRAL) )
    {
        if (victim == ch)
            send_to_char("You already perceive the Astral plane.\n\r",ch);
        else
            act("$N can already perceive the Astral plane.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_ASTRAL;
    af.where     = TO_AFFECTS;
    affect_to_char( victim, &af );
    send_to_char( "Your mind shifts into the Astral plane.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_pacify(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *rch;
    
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( saves_spell(rch, ch, level, DAM_MENTAL) )
        {
            send_to_char("You failed to pacify!\n\r",ch);     
            return;
        }
        else
        {
            if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
                REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
            send_to_char( "Your opponent is no longer aggressive.\n\r", ch );
            act("$n seems to have lost $s fighting edge",rch,NULL,NULL,TO_ROOM);
            return;
        }
    }
    return;
}

void spell_feeblemind ( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED( victim, AFF_FEEBLEMIND) )
    {
        act("I think $E's already dumb enough.", ch, NULL, victim, TO_CHAR);
        return;
    }
    
    if ( saves_spell(victim, ch, level, DAM_MENTAL) )
    {
        act("Could it be that $N's brain isn't as pathetic as we thought?",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.type      = sn;
    af.where     = TO_AFFECTS;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_INT;
    af.modifier  = -1 * (level / 2);
    af.bitvector = AFF_FEEBLEMIND;
    affect_to_char( victim, &af );
    send_to_char( "Hard . . . to . . . think . . .\n\r", victim );
    act("$n seems even dumber than usual!",victim,NULL,NULL,TO_ROOM);
    return;
}

/* fear, divine light, and holy binding */
/* Original Code by Jason Huang (god@sure.net).                       */
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */ 

void spell_fear( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_FEAR) )
    {
        act( "$N looks already scared to death!", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( saves_spell(victim, ch, level, DAM_MENTAL) )
    {
	act( "$n tries to look scary but looks rather funny.",
	     ch, NULL, victim, TO_VICT );
        send_to_char( "Apparently you aren't too scary.\n\r", ch );
        return;
    }
    else
    {
	AFFECT_DATA af;

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = get_duration(sn, level);
	af.modifier  = -level/4;
	af.location  = APPLY_DIS;
	af.bitvector = AFF_FEAR;
	affect_to_char( victim, &af );

        act( "$N looks really scared!", ch, NULL, victim, TO_CHAR );
        act( "Ahhhhhh! Run away! Run away!", ch, NULL, victim, TO_VICT );
	return;
    }
    return;
}

void spell_divine_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *light;
    
    light = create_object( get_obj_index( OBJ_VNUM_DIVINE_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n invokes the gods and $p appears.",   ch, light, NULL, TO_ROOM );
    act( "You invoke the gods and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}


void spell_holy_binding ( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj;
    obj = (OBJ_DATA *) vo; 
    
    if (saves_dispel(level,obj->level,0))
    {
        send_to_char( "You fail to bind.\n\r", ch );
        return;
    }
    
    if (target == TARGET_OBJ)
    {
        if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
        {
            act("$p is already cursed.",ch,obj,NULL,TO_CHAR);
            return;
        }
        else if (IS_OBJ_STAT(obj,ITEM_NODROP))
        {
            SET_BIT(obj->extra_flags,ITEM_NOREMOVE);
            act("$p glows red.",ch,obj,NULL,TO_ALL);
            return;
        }
        else if (IS_OBJ_STAT(obj,ITEM_NOREMOVE))
        {
            SET_BIT(obj->extra_flags,ITEM_NODROP); 
            act("$p glows red.",ch,obj,NULL,TO_ALL);
            return;
        }
        else 
        {
            SET_BIT(obj->extra_flags,ITEM_NODROP);
            SET_BIT(obj->extra_flags,ITEM_NOREMOVE);
            act("$p glows red.",ch,obj,NULL,TO_ALL);
            return;
        }
    }
    else 
        act("This spell can only be cast on objects.",ch,obj,NULL,TO_CHAR);
    return;
}

void spell_damned_blade( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj;
    
    if((obj = get_eq_char(victim,WEAR_WIELD)) == NULL)
    {
	if ( ch == victim )
	    act( "You don't wield a weapon.", ch, NULL, victim, TO_CHAR );
	else
	    act( "$N doesn't wield a weapon.", ch, NULL, victim, TO_CHAR );

        return;
    }

    if (IS_SET(obj->extra_flags,ITEM_NOREMOVE) && IS_SET(obj->extra_flags,ITEM_NODROP))
    {
        if ( ch == victim )
            act( "Your weapon is already cursed.", ch, NULL, victim, TO_CHAR );
        else
            act( "$N's weapon is already cursed.", ch, NULL, victim, TO_CHAR );

        return;
    }

    if (!saves_dispel(level,obj->level,0))
    {
        SET_BIT(obj->extra_flags,ITEM_NODROP);
        SET_BIT(obj->extra_flags,ITEM_NOREMOVE);
        act("$p is imbued with a curse.",victim,obj,NULL,TO_CHAR);
        act("$p is imbued with a curse.",victim,obj,NULL,TO_ROOM);
    }
    else
    {
        act("$p resists the curse.",victim,obj,NULL,TO_ROOM);
        act("$p resists the curse.",victim,obj,NULL,TO_CHAR);
    }
    
    return;
}

void spell_turn_undead( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    AFFECT_DATA af;
    int dam;
    
    act( "You call to the gods for aid against the undead.\n\r", ch, NULL, NULL, TO_CHAR );
    act( "$n calls to the gods for aid against the undead.\n\r", ch, NULL, NULL, TO_ROOM );

    dam = get_sn_damage( sn, level, ch ) * AREA_SPELL_FACTOR * (1000 + ch->alignment) / 1000;
    
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( is_safe_spell(ch, vch, TRUE) || !IS_UNDEAD(vch) )
            continue;

        check_killer(ch, vch);

        if ( IS_EVIL(ch) )
        {   /* Evil chars charm undead   */ 
            if ( IS_AFFECTED(vch, AFF_CHARM) || ch->fighting == vch )
                continue;
            spell_charm_person( gsn_charm_person, level, ch, (void*) vch, TARGET_CHAR );
        }
        else if (IS_GOOD(ch))
        {   /* Good chars harm undead */
            if ( saves_spell(vch, ch, level, DAM_HOLY) )
                full_dam( ch, vch, dam/2, sn, DAM_HOLY, TRUE );
            else
                full_dam( ch, vch, dam, sn, DAM_HOLY, TRUE );
        }
        else
        {   /* Neutral chars fear undead */
            if ( IS_AFFECTED(vch, AFF_FEAR) )
                continue;
            spell_fear( gsn_fear, level, ch, (void*) vch, TARGET_CHAR );
        }

        if ( !IS_DEAD(ch) && !IS_DEAD(vch) && !vch->fighting && !is_safe_spell(vch, ch, FALSE) )
            set_fighting(vch, ch);
    }
}

void spell_necrosis ( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if (IS_AFFECTED(victim, AFF_NECROSIS) 
	|| IS_AFFECTED(victim, AFF_PLAGUE)
	|| saves_spell(victim, ch, level, DAM_DISEASE)
	|| (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
        if (ch == victim)
            send_to_char("You stave off illness.\n\r",ch);
        else
            act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    drop_align( ch );
    
    af.where     = TO_AFFECTS;
    af.type       = gsn_necrosis;
    af.level      = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_STR;
    af.modifier  = -30; 
    af.bitvector = AFF_PLAGUE;
    affect_to_char(victim,&af);
    af.location  = APPLY_AGI;
    af.bitvector = AFF_NECROSIS;
    affect_to_char(victim,&af);
    
    send_to_char ("You are torn apart by disease.\n\r",victim);
    act("$n bleeds out all over the place.", victim,NULL,NULL,TO_ROOM);
}

void spell_dominate_soul( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int align_change;
    
    if ( ch == victim )
    {
	send_to_char( "You reassure yourself that you're following the right path.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_WAR) )
    {
	send_to_char( "Your're here to fight, not to missionar!\n\r", ch );
	return;
    }

    /*
    if ( IS_NPC(victim) )
    {
	act( "You can't influence $N.", ch, NULL, victim, TO_CHAR );
	return;
    }
    */

    if ( (IS_NPC(victim) || IS_SET(victim->act, PLR_NOCANCEL))
	  && saves_spell(victim, ch, level, DAM_MENTAL) )
    {
        act("$n is toying with your mind.", ch, NULL, victim, TO_VICT );
        act("You fail to influence $N.",ch,NULL,victim,TO_CHAR);
	return;
    }

    act("$n indoctrinates you.", ch, NULL, victim, TO_VICT );
    act("You indoctrinate $N.",ch,NULL,victim,TO_CHAR);
    act("$N is dominated by $n.",ch,NULL,victim,TO_NOTVICT);

    align_change = (ch->alignment - victim->alignment) / 10;
    if ( align_change > 0 )
	align_change = UMIN( level, align_change );
    else
	align_change = UMAX( -level, align_change );

    change_align( victim, align_change );
    if ( align_change > 0 )
	change_align( ch, (-align_change) / 4 );
}


void spell_animate_dead( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    OBJ_DATA *cor;
    CHAR_DATA *mob;
    CHAR_DATA *check;
    char buf[MAX_STRING_LENGTH];
    int mlevel, chance;
    int puppet_skill = get_skill( ch, gsn_puppetry );

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_WAR) )
    {
	send_to_char( "Go fight yourself!\n\r", ch );
	return;
    }

    send_to_char( "You attempt to wake the dead!\n\r", ch );
    act( "$n performs an unholy ritual to wake the dead.", ch, NULL, NULL, TO_ROOM );
    
    cor = get_obj_list( ch, "corpse", ch->in_room->contents );
    
    if (cor == NULL) 
    {
        send_to_char( "Nothing happens.\n\r",ch);
        return;
    }
    
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE) || IS_SET(ch->in_room->room_flags,ROOM_LAW))
    {
        send_to_char("Not in this room.\n\r",ch);
        return;
    }
    
    if (cor->level <= level)
        mlevel = (level + cor->level * 2) / 4;
    else
        mlevel = (level * 2 + cor->level) / 4;    
    /* bonus for puppetry skill */
    mlevel = URANGE(1, mlevel, ch->level) * (1000 + puppet_skill) / 1000;
    mlevel += mastery_bonus(ch, gsn_puppetry, 3, 5);
    
    /* Check number of charmees against cha */
    if ( check_cha_follow(ch, mlevel) < mlevel )
        return;
    
    chance = 100 + (level - cor->level) / 4;
    
    if ( number_percent() > chance ) 
    {
        send_to_char( "Nothing happens.\n\r", ch);
        return;
    }
    
    if ((mob = create_mobile(get_mob_index(MOB_VNUM_ZOMBIE)))==NULL) 
        return;
    
    check_improve( ch, gsn_puppetry, TRUE, 1 );
    
    if ( number_percent() <= puppet_skill )
    {
        SET_BIT( mob->off_flags, OFF_RESCUE );
        REMOVE_AFFECT( mob, AFF_SLOW );
    }
    
    set_mob_level( mob, mlevel );

    sprintf(buf,"%sThis zombie was reincarnated by the might of %s.\n\r\n\r",
        mob->description,ch->name);
    free_string(mob->description);
    mob->description = str_dup(buf);
    
    char_to_room( mob, ch->in_room );

    /* wear eq from corpse */
    get_eq_corpse( mob, cor );
    do_wear( mob, "all" );
    
    extract_obj (cor);
    change_align(ch, -10);
    
    /* Added new spell effect to destroy zombie after timer. -Rim 3/20/98 */
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level) * (100+puppet_skill) / 100;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_ANIMATE_DEAD;
    affect_to_char( mob, &af );
    
    if ( number_percent() > chance ) 
    {
        send_to_char( "You raise a zombie and it turns on you!\n\r", ch );
        act( "$n raises a zombie which attacks!", ch, NULL, NULL, TO_ROOM );
        mob_hit( mob, ch, TYPE_UNDEFINED );
    } 
    else 
    {
        send_to_char( "You raise a zombie and it follows you.\n\r", ch );
        act( "$n raises a zombie follower.", ch, NULL, NULL, TO_ROOM );
        add_follower( mob, ch );
        mob->leader = ch;
        af.bitvector = AFF_CHARM;
        affect_to_char( mob, &af );
    }
    return;
}

void spell_ghost_chant( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *mob;
    char buf[MAX_STRING_LENGTH];
    int mlevel;
    int puppet_skill = get_skill( ch, gsn_puppetry );

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_WAR) )
    {
        send_to_char( "Go fight yourself!\n\r", ch );
        return;
    }

    send_to_char( "You call the spirits of the dead!\n\r", ch );
    act( "$n performs an unholy chant to call the spirits of the dead.", ch, NULL, NULL, TO_ROOM );
    
    mlevel = level / 2;
    /* bonus for puppetry skill */
    mlevel = URANGE(1, mlevel, ch->level) * (1000 + puppet_skill) / 1000;
    mlevel += mastery_bonus(ch, gsn_puppetry, 3, 5);
    
    /* Check number of charmees against cha */
    if ( check_cha_follow(ch, mlevel) < mlevel )
        return;
   
    if ( (mob = create_mobile(get_mob_index(MOB_VNUM_SPIRIT))) == NULL ) 
        return;
    
    check_improve( ch, gsn_puppetry, TRUE, 1 );
    
    if ( per_chance(puppet_skill) )
    {
        SET_BIT(mob->off_flags, OFF_RESCUE);
        SET_BIT(mob->off_flags, OFF_FAST);
    }
    
    set_mob_level( mob, mlevel );

    sprintf(buf,"%sThis spirit was bound by the might of %s.\n\r\n\r", mob->description, ch->name);
    free_string(mob->description);
    mob->description = str_dup(buf);
    
    char_to_room( mob, ch->in_room );
    change_align(ch, -10);
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level) * (100+puppet_skill) / 100;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );

    add_follower( mob, ch );
    mob->leader = ch;

    return;
}

void spell_cannibalism( int sn, int level, CHAR_DATA *ch, void *vo ,int target)
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_WAR) )
    {
	send_to_char( "You can't do that during war!\n\r", ch );
	return;
    }

    if (ch->hit > 100)
    {
	send_to_char( "You draw magic energy from your life force.\n\r", ch );
        ch->mana = UMIN( ch->mana + 70, ch->max_mana );
        ch->hit -= 100;
    } 
    else send_to_char ("You haven't enough life left.\n\r",ch); 
    return;
}

void spell_ritual_sacrifice ( int sn, int level, CHAR_DATA *ch, void *vo ,int target)
{
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch, AFF_RITUAL) )
    {
	send_to_char( "The ritual is ready, you're just missing a corpse!\n\r", ch );
	return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_NONE;
    af.modifier  = 0; 
    af.bitvector = AFF_RITUAL;
    affect_to_char(ch,&af);
    
    send_to_char ("You prepare an evil sacrifice.\n\r",ch);
}

void spell_cure_mortal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;
    
    heal = get_sn_heal( sn, level, ch, victim ) * 2;
    heal = heal * (victim->max_hit - victim->hit) / victim->max_hit;
    /*heal += skill_table[sn].min_mana;*/
    heal = UMIN(heal, victim->max_hit - victim->hit);
    
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );

    if ( victim->max_hit == victim->hit )
    {
        send_to_char( "You feel excellent!\n\r", victim );
        if ( ch != victim )
            act( "$E is in excellent health.", ch, NULL, victim, TO_CHAR );
    }
    else
    {
        send_to_char( "You feel a lot better!\n\r", victim );
        if ( ch != victim )
            act( "$E looks a lot better!", ch, NULL, victim, TO_CHAR );
    }
    return;
}

void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int  heal;
    
    heal = get_sn_heal( sn, level, ch, victim );
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );

    if ( victim->max_hit == victim->hit )
    {
        send_to_char( "You feel excellent!\n\r", victim );
        if ( ch != victim )
            act( "$E is in excellent health.", ch, NULL, victim, TO_CHAR );
    }
    else
    {
        send_to_char( "You feel much better!\n\r", victim );
        if ( ch != victim )
            act( "$E looks much better!", ch, NULL, victim, TO_CHAR );
    }
    return;
}


void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;
    
    heal = get_sn_heal( sn, level, ch, victim );
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    if ( victim->max_hit == victim->hit )
    {
        send_to_char( "You feel excellent!\n\r", victim );
        if ( ch != victim )
            act( "$E is in excellent health.", ch, NULL, victim, TO_CHAR );
    }
    else
    {
        send_to_char( "You feel better!\n\r", victim );
        if ( ch != victim )
            act( "$E looks better!", ch, NULL, victim, TO_CHAR );
    }
    return;
}

void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;
    
    heal = get_sn_heal( sn, level, ch, victim );
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );

    if ( victim->max_hit == victim->hit )
    {
        send_to_char( "You feel excellent!\n\r", victim );
        if ( ch != victim )
            act( "$E is in excellent health.", ch, NULL, victim, TO_CHAR );
    }
    else
    {
        send_to_char( "You feel a bit better!\n\r", victim );
        if ( ch != victim )
            act( "$E looks a bit better.", ch, NULL, victim, TO_CHAR );
    }
    return;
}

void spell_minor_group_heal( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *gch = (CHAR_DATA *) vo;
    int heal;
    
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( !is_same_group( gch, ch ) )
            continue;
	heal = get_sn_heal( sn, level, ch, gch ) * 2/3;
        gch->hit = UMIN( gch->hit + heal, gch->max_hit );
        update_pos( gch );
        send_to_char( "You feel better!\n\r", gch );
	check_sn_multiplay( ch, gch, sn );
    }
    send_to_char( "Your magic assists your allies.\n\r", ch );
    return;
}

void spell_group_heal( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *gch = (CHAR_DATA *) vo;
    int heal;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( !is_same_group( gch, ch ) )
            continue;
	heal = get_sn_heal( sn, level, ch, gch ) * 2/3;
        gch->hit = UMIN( gch->hit + heal, gch->max_hit );
        update_pos( gch );
        send_to_char( "You feel better!\n\r", gch );
	check_sn_multiplay( ch, gch, sn );
    }
    send_to_char( "Your magic assists your allies.\n\r", ch );
    return;
}

void spell_major_group_heal( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *gch = (CHAR_DATA *) vo;
    int heal;
    
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( !is_same_group( gch, ch ) )
            continue;
	heal = get_sn_heal( sn, level, ch, gch ) * 2/3;
        gch->hit = UMIN( gch->hit + heal, gch->max_hit );
        update_pos( gch );
        send_to_char( "You feel better!\n\r", gch );
	check_sn_multiplay( ch, gch, sn );
    }
    send_to_char( "Your magic assists your allies.\n\r", ch );
    return;
}

void spell_restoration ( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal = victim->max_hit - victim->hit;
    double factor = 2.0;

    if ( !was_obj_cast )
    {
        int skill = get_skill(ch, gsn_anatomy) + mastery_bonus(ch, gsn_anatomy, 15, 25);
        factor += factor * get_skill(ch, gsn_anatomy) / 200;
        check_improve(ch, gsn_anatomy, TRUE, 1);
    }
    if ( ch != victim )
        factor += factor / 3;
    factor *= 100.0 / mastery_adjust_cost(100, get_mastery(ch, sn));

    if ( ch->mana < heal/factor )
	heal = ch->mana * factor;

    ch->mana -= heal/factor;
    victim->hit = UMIN(victim->hit + heal, victim->max_hit );
    update_pos( victim );

    if ( victim->max_hit <= victim->hit )
    {
        send_to_char( "You feel excellent!\n\r", victim );
        if ( ch != victim )
            act( "$E is fully healed.", ch, NULL, victim, TO_CHAR );
    }
    else
    {
       send_to_char( "Your health is partially restored!\n\r", victim );
       if ( ch != victim )
            act( "You restore some of $N's health.", ch, NULL, victim, TO_CHAR );
    }
    return;
}

void spell_hand_of_siva( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *weapon;
    int i, weapon_level;
    char buf[MAX_STRING_LENGTH];
    char *arg, *arg2;
    bool weapon_2hands = FALSE;

    arg = one_argument( target_name, buf ); // weapon name
    for ( i = 0; weapon_class[i].name != NULL; i++) 
        if ( !str_cmp( buf, weapon_class[i].name) ) break;
        
    if (weapon_class[i].name==NULL || buf[0]=='\0')
    {
	send_to_char( "Syntax:  cast 'hand of siva' <weapon_type>\n\r"
	  "         cast 'hand of siva' <weapon_type> <level>\n\r"
	  "         cast 'hand of siva' <weapon_type> twohands <level>\n\r", ch );
	return;
    }

    arg2 = one_argument( arg, buf );
    if ( buf[0] != '\0' && !str_prefix( buf, "twohands" ) )
    {
	weapon_2hands = TRUE;
	if( arg2[0] != '\0' )
	    arg2 = one_argument( arg2, buf ); // level
    }

    if ( buf[0] == '\0' )
	weapon_level = UMIN( level, ch->level );
    else
    {
	if ( !is_number(buf) )
	{
	    send_to_char( "Syntax:  cast 'hand of siva' <weapon_type>\n\r"
			  "         cast 'hand of siva' <weapon_type> <level>\n\r"
			  "         cast 'hand of siva' <weapon_type> twohands <level>\n\r", ch );
	    return;
	}
	weapon_level = atoi( buf );
	if ( weapon_level > level )
	{
	    send_to_char( "Weapon level can't be higher than spell level!\n\r", ch );
	    weapon_level = level;
	}
    }

    weapon = create_object( get_obj_index( OBJ_VNUM_SIVA_WEAPON ), 0 );
        
    sprintf( buf, weapon->name, weapon_class[i].name );
    free_string( weapon->name );
    weapon->name = str_dup( buf );

    sprintf( buf, weapon->short_descr, weapon_class[i].name );
    free_string( weapon->short_descr );
    weapon->short_descr = str_dup( buf );
    
    sprintf( buf, weapon->description, weapon_class[i].name );
    free_string( weapon->description );
    weapon->description = str_dup( buf );
    
    weapon->level=weapon_level;
    weapon->value[0] = weapon_class[i].bit ;
    
    if ( weapon_2hands )
        I_SET_BIT( weapon->value[4], WEAPON_TWO_HANDS );
    else
        I_REMOVE_BIT( weapon->value[4], WEAPON_TWO_HANDS );
    
    set_weapon_dam( weapon, weapon_dam_spec(weapon_level, weapon_2hands) );

    obj_to_room( weapon, ch->in_room );
    act( "$p suddenly appears!", ch, weapon, NULL, TO_ROOM );
    act( "$p suddenly appears!", ch, weapon, NULL, TO_CHAR );
    
    return;
}

void spell_goodberry( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    OBJ_DATA *berry;
    char buf[MIL];
    int berry_level;

    /* get weapon level */
    one_argument( target_name, buf ); // level
    if ( buf[0] == '\0' )
	berry_level = UMIN( level, ch->level );
    else
    {
	if ( !is_number(buf) )
	{
	    send_to_char( "Berry level must be a number!\n\r", ch );
	    return;
	}
	berry_level = atoi( buf );
	if ( berry_level > level )
	{
	    send_to_char( "Berry level can't be higher than spell level!\n\r", ch );
	    berry_level = level;
	}
	if (berry_level <= 0)
	{
	  send_to_char("These are supposed to heal, not kill!\n\r", ch);
	  return;
	}
    }
    berry = create_object( get_obj_index( OBJ_VNUM_GOODBERRY ), 0 );
    berry->value[0] = berry_level;
    berry->level = berry_level;
    obj_to_room( berry, ch->in_room );
    act( "$p suddenly appears.", ch, berry, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, berry, NULL, TO_CHAR );
    return;
}

void spell_protection_magic(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_PROTECT_MAGIC))
    {
        if (victim == ch)
            send_to_char("You are already protected.\n\r",ch);
        else
            act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_SAVES;
    af.modifier  = -5;
    af.bitvector = AFF_PROTECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "You feel safe from magic.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from magic.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_immolation(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_ELEMENTAL_SHIELD))
    {
        if (victim == ch)
        {
            send_to_char("Your shield shimmers away as an aura of fire protects you.\n\r",ch);
            affect_strip_flag(ch, AFF_ELEMENTAL_SHIELD);

            af.where     = TO_AFFECTS;
            af.type      = sn;
            af.level     = level;
            af.duration  = get_duration(sn, level);
            af.location  = APPLY_AC;
            af.modifier  = -level/8;
            af.bitvector = AFF_ELEMENTAL_SHIELD;
            affect_to_char( victim, &af );
    
            if (!IS_SET(victim->res_flags, RES_FIRE))
            {
                af.where = TO_RESIST;
                af.type = sn;
                af.level = level;
                af.duration = get_duration(sn, level);
                af.location = APPLY_NONE;
                af.modifier=0;
                af.bitvector=RES_FIRE;
                affect_to_char(victim, &af);
            }
        }
        else
            act("$N is already shielded.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_AC;
    af.modifier  = -level/8;
    af.bitvector = AFF_ELEMENTAL_SHIELD;
    affect_to_char( victim, &af );
    
    if (!IS_SET(victim->res_flags, RES_FIRE))
    {
        af.where = TO_RESIST;
        af.type = sn;
        af.level = level;
        af.duration = get_duration(sn, level);
        af.location = APPLY_NONE;
        af.modifier=0;
        af.bitvector=RES_FIRE;
        affect_to_char(victim, &af);
    }
    
    send_to_char( "You are surrounded by an aura of fire.\n\r", victim );
    if ( ch != victim )
        act("$N is surrounded by an aura of fire.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_epidemic(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_ELEMENTAL_SHIELD))
    {
        if (victim == ch)
        {
            send_to_char("Your shield shimmers away as an aura of disease protects you.\n\r",ch);
            affect_strip_flag(ch, AFF_ELEMENTAL_SHIELD);

            af.where     = TO_AFFECTS;
            af.type      = sn;
            af.level     = level;
            af.duration  = get_duration(sn, level);
            af.location  = APPLY_AC;
            af.modifier  = -level/8;
            af.bitvector = AFF_ELEMENTAL_SHIELD;
            affect_to_char( victim, &af );
    
            if (!IS_SET(victim->res_flags, RES_DISEASE))
            {
                af.where = TO_RESIST;
                af.type = sn;
                af.level = level;
                af.duration = get_duration(sn, level);
                af.location = APPLY_NONE;
                af.modifier=0;
                af.bitvector=RES_DISEASE;
                affect_to_char(victim, &af);
            }
        }
        else
            act("$N is already shielded.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_AC;
    af.modifier  = -level/8;
    af.bitvector = AFF_ELEMENTAL_SHIELD;
    affect_to_char( victim, &af );
    
    if (!IS_SET(victim->res_flags, RES_DISEASE))
    {
        af.where = TO_RESIST;
        af.type = sn;
        af.level = level;
        af.duration = get_duration(sn, level);
        af.location = APPLY_NONE;
        af.modifier=0;
        af.bitvector=RES_DISEASE;
        affect_to_char(victim, &af);
    }
    
    send_to_char( "Disease courses through your veins.\n\r", victim );
    if ( ch != victim )
        act("Disease courses through $N's veins.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_electrocution(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
   if ( IS_AFFECTED(victim, AFF_ELEMENTAL_SHIELD))
    {
        if (victim == ch)
        {
            send_to_char("Your shield shimmers away as an aura of electricity protects you.\n\r",ch);
            affect_strip_flag(ch, AFF_ELEMENTAL_SHIELD);

            af.where     = TO_AFFECTS;
            af.type      = sn;
            af.level     = level;
            af.duration  = get_duration(sn, level);
            af.location  = APPLY_AC;
            af.modifier  = -level/8;
            af.bitvector = AFF_ELEMENTAL_SHIELD;
            affect_to_char( victim, &af );
    
            if (!IS_SET(victim->res_flags, RES_LIGHTNING))
            {
                af.where = TO_RESIST;
                af.type = sn;
                af.level = level;
                af.duration = get_duration(sn, level);
                af.location = APPLY_NONE;
                af.modifier=0;
                af.bitvector=RES_LIGHTNING;
                affect_to_char(victim, &af);
            }
        }
        else
            act("$N is already shielded.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_AC;
    af.modifier  = -level/8;
    af.bitvector = AFF_ELEMENTAL_SHIELD;
    affect_to_char( victim, &af );
    
    if (!IS_SET(victim->res_flags, RES_LIGHTNING))
    {
        af.where = TO_RESIST;
        af.type = sn;
        af.level = level;
        af.duration = get_duration(sn, level);
        af.location = APPLY_AC;
        af.modifier=0;
        af.bitvector=RES_LIGHTNING;
        affect_to_char(victim, &af);
    }
    
    send_to_char( "Electricity runs through your body.\n\r", victim );
    if ( ch != victim )
        act("Electricity runs through $N's body.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_absolute_zero(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
   if ( IS_AFFECTED(victim, AFF_ELEMENTAL_SHIELD))
    {
        if (victim == ch)
        {
            send_to_char("Your shield shimmers away as an aura of frost protects you.\n\r",ch);
            affect_strip_flag(ch, AFF_ELEMENTAL_SHIELD);

            af.where     = TO_AFFECTS;
            af.type      = sn;
            af.level     = level;
            af.duration  = get_duration(sn, level);
            af.location  = APPLY_AC;
            af.modifier  = -level/8;
            af.bitvector = AFF_ELEMENTAL_SHIELD;
            affect_to_char( victim, &af );
    
            if (!IS_SET(victim->res_flags, RES_COLD))
            {
                af.where = TO_RESIST;
                af.type = sn;
                af.level = level;
                af.duration = get_duration(sn, level);
                af.location = APPLY_NONE;
                af.modifier=0;
                af.bitvector=RES_COLD;
                affect_to_char(victim, &af);
            }
        }
        else
            act("$N is already shielded.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_AC;
    af.modifier  = -level/8;
    af.bitvector = AFF_ELEMENTAL_SHIELD;
    affect_to_char( victim, &af );
    
    if (!IS_SET(victim->res_flags, RES_COLD))
    {
        af.where = TO_RESIST;
        af.type = sn;
        af.level = level;
        af.duration = get_duration(sn, level);
        af.location = APPLY_NONE;
        af.modifier=0;
        af.bitvector=RES_COLD;
        affect_to_char(victim, &af);
    }
    
    send_to_char( "You are surrounded by a shield of ice.\n\r", victim );
    if ( ch != victim )
        act("$N is surrounded by a shield of ice.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_fade(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(ch, AFF_FADE) || IS_AFFECTED(ch, AFF_MINOR_FADE))
    {
        send_to_char("You are already fading out of existence.\n\r",ch);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FADE;
    affect_to_char( victim, &af );
    send_to_char( "You begin to phase in and out of existence.\n\r", victim );
    act("$n begins to phase in and out of existence.", victim, NULL, NULL, TO_ROOM);
}


void spell_breathe_water(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_BREATHE_WATER))
    {
        if (victim == ch)
            send_to_char("You already have gills.\n\r",victim);
        else
            act("$N already has gills.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_BREATHE_WATER;
    affect_to_char( victim, &af );
    send_to_char( "You grow gills on your neck.\n\r", victim );
    act("$n grows a set of gills.", victim, NULL, NULL, TO_ROOM);
}


void spell_monsoon( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
				
    if (!IS_OUTSIDE(ch) )
    {
        send_to_char( "A monsoon INDOORS?  I think not.\n\r", ch );
        return;
    }
    if (weather_info.sky < SKY_RAINING )
    {
        send_to_char ( "The weather is much too nice for that!", ch );
        return;
    }
    dam = get_sn_damage( sn, level, ch) * AREA_SPELL_FACTOR * 1.5;
    
    send_to_char( "A torrent of rain drenches your foes!\n\r", ch );
    act( "$n calls down a monsoon to drench $s enemies!!!", ch, 
        NULL, NULL, TO_ROOM );
    
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

        if ( !is_safe_spell(ch, vch, TRUE) )
        {
            if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : TRUE ))
                full_dam( ch, vch,
                saves_spell(vch, ch, level, DAM_DROWNING) ? dam / 2 : dam,
                sn,DAM_DROWNING,TRUE);
            continue;
        }
    }
    return;
}

void spell_hailstorm( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int hailstone,dam,dice_nr;
    int max = get_sn_damage( sn, level, ch ) / 100;
    max = UMAX( 1, max );
    
    if (!IS_OUTSIDE(ch) )
    {
        send_to_char( "A hailstorm indoors? I think not.\n\r", ch );
        return;
    }
    
    /*
    if ( weather_info.sky < SKY_RAINING )
    {
        send_to_char( "The weather is MUCH too nice for that!\n\r", ch );
        return;
    }
    */
    /* the worse the weather, the more powerful */
    dice_nr = 9 + weather_info.sky;
    
    send_to_char( "Hailstones pelt down upon the heads of your foes!\n\r", ch );
    send_to_char( "Hailstorms pelt down upon your head!\n\r", victim );
    
    for (hailstone=0; hailstone<max; hailstone++)
    {
        dam = dice( dice_nr, 19 );
        if ( saves_spell(victim, ch, level, DAM_COLD) )
            dam /= 2;
        full_dam( ch, victim, dam, sn, DAM_COLD ,TRUE);
	CHECK_RETURN(ch, victim);
    }
    return;
}



void spell_meteor_swarm( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int meteor,dam;
    int max = get_sn_damage( sn, level, ch) / 200;
    max = UMAX( 1, max );
    
    if (!IS_OUTSIDE(ch) )
    {
        send_to_char( "But you are indoors!\n\r", ch );
        return;
    }
    
    for (meteor=0; meteor<max; meteor++)
    {
        dam = dice( 22, 19 );
        
        if (meteor%2==0)
        {
            if ( saves_spell(victim, ch, level, DAM_FIRE) )
                dam /= 2;
            full_dam( ch, victim, dam, sn, DAM_FIRE ,TRUE);
        }
        else
        {
            if ( saves_spell(victim, ch, level, DAM_HARM) )
                dam /= 2;
            full_dam( ch, victim, dam, sn, DAM_HARM, TRUE);
        }
	CHECK_RETURN(ch, victim);
    }
    return;
}

/*Some New Spells by Siva*/
void spell_entangle ( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( is_affected( victim, sn ) )
    {
        act("$E's already entangled.", ch, NULL, victim, TO_CHAR);
        return;
    }
    
    if (IS_SET(ch->in_room->room_flags,ROOM_INDOORS))
		  {
        send_to_char("It's pointless to cast entangle indoors.\n\r",ch);
        return;
		  }
    
    if ( ch->in_room->sector_type == SECT_CITY)
    {
        send_to_char("There's too little vegetation in the city.\n\r",ch);
        return; 
    }
    
    if ((ch->in_room->sector_type >= SECT_WATER_SHALLOW) &&
        (ch->in_room->sector_type <= SECT_UNDERWATER))
    {
        send_to_char("There's too little vegetation underwater.\n\r",ch);
        return; 
    }
    
    if (  ( ch->in_room->sector_type == SECT_DESERT) && ( (get_curr_stat(ch,STAT_LUC)/3) < number_percent()  ))
	   {
        send_to_char("You don't find any useable growth for your spell.\n\r",ch);
        return; 
	   }
    
       /*
       switch(ch->in_room->sector_type)
       {
       case(SECT_DESERT):
       send_to_char("The desert scrub is too weak to entangle.\n\r",ch);
       return;
       break;
       case(SECT_CITY):
       send_to_char("There's too little vegetation in the city.\n\r",ch);
       return;   
       break;
       case(SECT_MOUNTAIN):
       send_to_char("The mountains provide too little vegetation to entangle.\n\r",ch);
       return;
       break;        
       case(SECT_INSIDE):
       send_to_char("Houseplants are hardly enough to entangle your foe.\n\r",ch);
       return;
       break;        
       case(SECT_WATER_SWIM):
       case(SECT_WATER_NOSWIM):
       send_to_char("Seaweed is to weak to entangle.\n\r",ch);
       return;
       break;        
       case(SECT_AIR):
       send_to_char("You'll find more vegetation nearer to the earth.\n\r",ch);
       return;
       break;
       case(SECT_FIELD):
       case(SECT_FOREST):
       case(SECT_HILLS):
       break;
       default:
       send_to_char("There's too little plant growth to entangle.\n\r",ch);
       return;
       break;
       }
    */
    
    if ( saves_spell(victim, ch, level, DAM_ENERGY ) )
    {
        act("$N evades your entangling flora!",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_AGI;
    af.modifier  = -1 * (level / 2);
    af.bitvector = AFF_ENTANGLE;
    affect_to_char( victim, &af );
    if (IS_AFFECTED(victim,AFF_HASTE))               
        REMOVE_BIT(victim->affect_field,AFF_HASTE);
    send_to_char( "Nearby plant life shudders to life and entangles you!\n\r", victim );
    act("$n is entangled!",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_pass_without_trace( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_NO_TRACE) )
    {
        if (victim == ch)
            send_to_char("Your path is already obscured.\n\r",ch);
        else
            act("$N is already leaving no trace.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_NO_TRACE;
    affect_to_char( victim, &af );
    act( "$n's trail is becoming obscured.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You can travel without leaving a trail now.\n\r", victim );
    return;
}

void spell_tree_golem( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    CHAR_DATA *mob;
    CHAR_DATA *check;
    char buf[MAX_STRING_LENGTH];
    int mlevel, mhp, chance;
    int charmed, max;
    int beast_skill = get_skill(ch, gsn_beast_mastery);
    
    if ( ch->in_room->sector_type != SECT_FOREST)
    {
        send_to_char("Need to be in a forest for this spell to work.\n\r",ch);
        return; 
    }
    
    if ( IS_SET( ch->act, PLR_WAR ) )
    {
        send_to_char( "This war does not concern the woodland spirits.\n\r", ch );
        return;
    }
    
    act( "$n tries to summon a woodland spirit into a nearby tree.", ch, NULL, NULL, TO_ROOM );
    
    /* Check number of charmees against cha*/ 
    mlevel = (6*level + beast_skill) / 8;
    mlevel = URANGE(1, mlevel, ch->level);
    if ( check_cha_follow(ch, mlevel) < mlevel )
        return;
   
    if ((mob = create_mobile(get_mob_index(MOB_VNUM_TREEGOLEM)))==NULL) 
        return;
    
    set_mob_level( mob, mlevel );

    sprintf(buf,"%s\n\rA tree springs to life and follows %s.\n\r\n\r",
        mob->description,ch->name);
    free_string(mob->description);
    mob->description = str_dup(buf);
    
    char_to_room( mob, ch->in_room );
    
    send_to_char( "A woodland spirit imbues a tree with life!\n\r", ch );
    act( "$n's spells gives life to a tree golem!", ch, NULL, NULL, TO_ROOM );
    add_follower( mob, ch );
    mob->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level   = level;
    af.duration  = get_duration(sn, level) * (100 + beast_skill) / 100;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );
    
    return;
}

void spell_water_elemental( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *mob;
    MOB_INDEX_DATA *mobIndex;
    char buf[MAX_STRING_LENGTH];
    char liquid_name[MAX_STRING_LENGTH];
    int mlevel;
    int sector = ch->in_room->sector_type;
    
    mlevel = URANGE(1, level * 3/4, ch->level);
    sprintf(liquid_name, "water");

    if ( sector != SECT_WATER_SHALLOW
        && sector != SECT_WATER_DEEP
        && sector != SECT_UNDERWATER )
    {
        // can summon from fountain (e.g. create spring), but level will be lower
        OBJ_DATA *fountain = get_obj_by_type(ch->in_room->contents, ITEM_FOUNTAIN);
        if ( fountain != NULL )
        {
            mlevel = URANGE(1, level * 2/3, ch->level);
            int liquid = UMAX(0, fountain->value[2]);
            sprintf(liquid_name, "%s", liq_table[liquid].liq_name);
        }
        else
        {
            send_to_char("You need water to summon water elementals.\n\r",ch);
            return;
        }
    }
    
    if ( IS_SET( ch->act, PLR_WAR ) )
    {
        send_to_char( "This war does not concern the elemental spirits.\n\r", ch );
        return;
    }
    
    /* Check number of charmees against cha*/ 
    if ( check_cha_follow(ch, mlevel) < mlevel )
        return;
       
    if ( (mobIndex = get_mob_index(MOB_VNUM_WATER_ELEMENTAL)) == NULL ) 
        return;
    mob = create_mobile(mobIndex);
    
    set_mob_level( mob, mlevel );

    // set name, description etc. to match liquid
#define rename(VAR) free_string(mob->VAR);mob->VAR=str_dup(buf)
    sprintf(buf, "%s elemental", liquid_name);
    rename(name);
    sprintf(buf, "A %s elemental", liquid_name);
    rename(short_descr);
    sprintf(buf, "A %s elemental flows along.", liquid_name);
    rename(long_descr);
    sprintf(buf, "%s has imbued this blob of %s with an elemental spirit.\n\r", ch->name, liquid_name);
    rename(description);
#undef rename
    
    char_to_room( mob, ch->in_room );
    
    act( "An elemental spirit imbues the $T with life!", ch, NULL, liquid_name, TO_CHAR );
    act( "$n's spell gives life to a $T elemental!", ch, NULL, liquid_name, TO_ROOM );
    add_follower( mob, ch );
    mob->leader  = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );
    
    return;
}

void spell_windwar( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
				
    if (!IS_OUTSIDE(ch) )
    {
        send_to_char( "The winds refuse to war indoors.\n\r", ch );
        return;
    }

    dam = get_sn_damage( sn, level, ch ) * AREA_SPELL_FACTOR;
    if ((ch->in_room->sector_type) == SECT_MOUNTAIN)     
    {
        send_to_char ( "The mountain winds make war with your foes!\n\r", ch );
    }
    else if (weather_info.sky == SKY_RAINING )         
    {
	send_to_char ( "The storm winds gather at your beckoning!\n\r", ch );
	dam = dam * 3/4;
    }
    else 
	dam = dam / 2;

    send_to_char( "The winds make war with your foes!\n\r", ch );
    act( "$n commands the wind to make war upon $s enemies!!!", ch, 
	 NULL, NULL, TO_ROOM );
    
    /* why check all chars? only check those in room! --Bobble */
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;
	
	if ( vch == ch || is_safe_spell(ch, vch, TRUE) )
	    continue;
	
	if (weather_info.sky == SKY_RAINING )    
	{
	    full_dam( ch, vch,
		    saves_spell(vch, ch, level ,DAM_DROWNING) ? dam / 2 : dam,
		    sn,DAM_DROWNING,TRUE);
	}   
	else
	{
	    full_dam( ch, vch,
		    saves_spell(vch, ch, level, DAM_BASH) ? dam / 2 : dam,
		    sn,DAM_BASH,TRUE);
	}   

	/*check for disarm*/ 
	if ( chance(10) )
	{
	    if ( disarm(ch, vch, TRUE, get_mastery(ch, sn)) )
	    {
            act( "$n's wind blows your weapon from your grasp!", ch, NULL, vch, TO_VICT );
            act( "Your winds disarm $N!", ch, NULL, vch, TO_CHAR );
            act( "$n's wind blow $N's weapon away!", ch, NULL, vch, TO_NOTVICT );
	    }
	}
	/*end disarm section*/
    }
}

/* Added by Tryste */
void spell_sticks_to_snakes( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    CHAR_DATA *mob;
    CHAR_DATA *check;
    char buf[MAX_STRING_LENGTH];
    int mlevel, chance;
    int snake_count, max_snake;
    int beast_skill = get_skill(ch, gsn_beast_mastery);
    
    if ( ch->in_room->sector_type != SECT_FOREST)
    {
        send_to_char("Need to be in a forest for this spell to work.\n\r",ch);
        return;
    }
    
    if ( IS_SET( ch->act, PLR_WAR ) )
    {
        send_to_char( "This war does not concern the woodland spirits.\n\r", ch);
        return;
    }
    act( "$n tries to raise snakes from sticks.", ch, NULL, NULL,
        TO_ROOM);

    /* Check number of charmees against cha*/
    mlevel = (5*level + beast_skill) / 10;
    mlevel = URANGE(1, mlevel, ch->level);
    max_snake = check_cha_follow( ch, mlevel );
    if ( max_snake < mlevel )
        return;
    
    chance = 100;
    snake_count = 0;
    while ( (snake_count + 1) * mlevel < max_snake && number_percent() <= chance ) {
        
        if ((mob = create_mobile(get_mob_index(MOB_VNUM_SNAKE)))==NULL)
            return;  
        
        set_mob_level( mob, mlevel );

        sprintf(buf,"%s\n\rA snake that was once a stick is following %s.\n\r\n\r", mob->description,ch->name);   
        free_string(mob->description);
        mob->description = str_dup(buf);
        
        char_to_room( mob, ch->in_room );
        
        add_follower( mob, ch );
        mob->leader = ch;
        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level   = level;
        af.duration  = get_duration(sn, level) * (100 + beast_skill) / 100;
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char( mob, &af );

        chance -= 20;
        snake_count += 1;
    }
    if (snake_count == 1)
        send_to_char("You wave your hand and a stick becomes a snake!\n\r", ch);
    else	    
    {
        sprintf(buf, "You wave your hand and %d sticks become enlivened and turn to snakes!\n\r", snake_count);
        send_to_char(buf, ch);
    }
    act( "$n's magic changes sticks to snakes!", ch, NULL, NULL, TO_ROOM);
    return;
}    

void spell_hand_of_god(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;
    RELIGION_DATA *rel;    
    
    /* first strike */
    if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) )
    {
	rel = get_religion(ch);
	if( rel == NULL )
	{
	    send_to_char("Your faith is not strong enough to call the wrath of the gods!\n\r",ch);
            return;
	}
	else if( !IS_BETWEEN( rel->min_align, ch->alignment, rel->max_align ) )
	{
	    send_to_char("Your god frowns upon you, and decides not to lend a hand.\n\r",ch);
	    return;
	}
    }

    if (is_safe(ch, victim))
        return;

    if( IS_EVIL(ch) )
    {
	if ( IS_IMMORTAL(ch) )
	{
	act("Divine fury leaps from $n's hand and arcs to $N.", ch,NULL,victim,TO_ROOM);
	act("Your fury leaps from your hand and arcs to $N.", ch,NULL,victim,TO_CHAR);
	act("Divine fury leaps from $n's hand and hits you!", ch,NULL,victim,TO_VICT);
	} else {
	act("The fury of the gods leaps from $n's hand and arcs to $N.", ch,NULL,victim,TO_ROOM);
	act("The fury of the gods leaps from your hand and arcs to $N.", ch,NULL,victim,TO_CHAR);
	act("The fury of the gods leaps from $n's hand and hits you!", ch,NULL,victim,TO_VICT);
	}
	deal_chain_damage( sn, level + 10, ch, victim, DAM_NEGATIVE );
    }
    else if ( IS_NEUTRAL(ch) )
    {
	if ( IS_IMMORTAL(ch) )
	{
	act("Divine power leaps from $n's hand and arcs to $N.", ch,NULL,victim,TO_ROOM);
	act("Your power leaps from your hand and arcs to $N.", ch,NULL,victim,TO_CHAR);
	act("Divine power leaps from $n's hand and hits you!", ch,NULL,victim,TO_VICT);
	} else {
	act("The power of the gods leaps from $n's hand and arcs to $N.", ch,NULL,victim,TO_ROOM);
	act("The power of the gods leaps from your hand and arcs to $N.", ch,NULL,victim,TO_CHAR);
	act("The power of the gods leaps from $n's hand and hits you!", ch,NULL,victim,TO_VICT);
	}
	deal_chain_damage( sn, level + 10, ch, victim, DAM_HARM );
    }
    else   // ch IS_GOOD
    {
	if ( IS_IMMORTAL(ch) )
	{
	act("Divine force leaps from $n's hand and arcs to $N.", ch,NULL,victim,TO_ROOM);
	act("Your holy force leaps from your hand and arcs to $N.", ch,NULL,victim,TO_CHAR);
	act("Divine force leaps from $n's hand and hits you!", ch,NULL,victim,TO_VICT);
	} else {
	act("The holy force of the gods leaps from $n's hand and arcs to $N.", ch,NULL,victim,TO_ROOM);
	act("The holy force of the gods leaps from your hand and arcs to $N.", ch,NULL,victim,TO_CHAR);
	act("The holy force of the gods leaps from $n's hand and hits you!", ch,NULL,victim,TO_VICT);
	}
	/* Good vs evil has a bonus because they have a penalty
        when casting upon a neutral or good-aligned foe. */
	if ( IS_EVIL(victim) )
	    deal_chain_damage( sn, level + 20, ch, victim, DAM_HOLY );
	else if ( IS_NEUTRAL(victim) )
	    deal_chain_damage( sn, level, ch, victim, DAM_HOLY );
	else
	    deal_chain_damage( sn, level/2, ch, victim, DAM_HOLY );
    }
}

void spell_laughing_fit( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED( victim, AFF_LAUGH ) ) 
    {
        if (victim == ch)
        {
            send_to_char("You are already laughing your ass off!\n\r",ch);
            return;
        }   
        else
        {
	    act("$E is laughing $S ass off already!", ch, NULL, victim, TO_CHAR );
            return;
        }
    }
    
    /* Laughing fit isn't checked in weaken, feeblemind, or soreness... so why
       should it be required to cast the spells in a certain order?
       Plus, this is used in quirkys insanity ... and I have just changed it so
       laughing fit is called more often than confusion in the stance, so this
       needed to be made more possible too :)
       Removed Aug 2003 by Quirky

    if ( IS_AFFECTED(victim, AFF_WEAKEN))
    {
        if (victim == ch)
        {
            send_to_char("You are too weak to laugh hard.\n\r",ch);
            return;
        }
        else
        {
            send_to_char("That person is too weak to laugh hard.\n\r",ch);
            return;
        }
    }
    
    if ( IS_AFFECTED(victim, AFF_FEEBLEMIND))
    {
        if (victim == ch)
        {
            send_to_char("You are too dumb to get even your own joke!\n\r", ch);
            return;
        }
        else
        {
            send_to_char("That person is too dumb to get your joke!\n\r", ch);
            return;
        }
    }
    
    if ( IS_AFFECTED(victim, AFF_SORE))
    {
        if (victim == ch)                
        {
            send_to_char("You are too sore to laugh!\n\r", ch);
            return;
        }
        else
        {
            send_to_char("That person is too sore to find that funny!\n\r", ch);
            return;
        }
    }
*/

    if ( saves_spell(victim, ch, level, DAM_MENTAL) )
    {
        send_to_char("Spell failed to have an effect.\n\r", ch );
        send_to_char("You find everything unusually funny for a moment.\n\r", victim );
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.modifier  = -2;
    af.bitvector = AFF_LAUGH;
    
    af.location  = APPLY_STR;
    affect_to_char(victim, &af);
    
    af.location  = APPLY_HITROLL;
    affect_to_char(victim, &af);
    
    af.location  = APPLY_INT;
    affect_to_char(victim, &af);
    
    send_to_char( "You begin to laugh uncontrollably!\n\r", victim );
    act("$n starts laughing like a madman.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_mass_confusion( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim;
    
    for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
    {
        if ( is_safe_spell(ch, victim, TRUE) || IS_AFFECTED(victim, AFF_INSANE) )
            continue;

	check_killer( ch, victim );

        if  ( saves_spell(victim, ch, level/2, DAM_MENTAL) )
	      /* || saves_spell(level,victim,DAM_CHARM)) */
        {
            if (ch == victim)
                send_to_char("{xYou feel momentarily {Ms{yi{Gl{Cl{Ry{x, but it passes.\n\r",ch);
            else
                act("$N seems to keep $S sanity.",ch,NULL,victim,TO_CHAR);
        }
	else
	{
	    af.where     = TO_AFFECTS;
	    af.type      = sn;
	    af.level     = level/2;
	    af.duration  = get_duration(sn, level);
	    af.location  = APPLY_INT;
	    af.modifier  = -15;
	    af.bitvector = AFF_INSANE;
	    affect_join(victim,&af);
        
	    send_to_char("{MY{bo{Cu{Gr {%{yw{Ro{mr{Bl{Cd{x {gi{Ys {%{ra{Ml{Bi{cv{Ge{x {yw{Ri{Mt{bh{%{wcolors{x{C?{x\n\r",victim);
	    act("$n giggles like $e lost $s mind.", victim,NULL,NULL,TO_ROOM);
	}

	if ( !IS_DEAD(ch) && !IS_DEAD(victim)
	     && victim->fighting == NULL
	     && !is_safe_spell(victim, ch, TRUE) )
	    multi_hit( victim, ch, TYPE_UNDEFINED );
    }
    return;
    
}

void spell_heroism( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    
    victim = (CHAR_DATA *) vo;
    
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim, AFF_HEROISM))
    {
        send_to_char("You are already a holy hero.\n\r",victim);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 10;
    af.bitvector = AFF_HEROISM;
    affect_to_char( victim, &af );
    af.location  = APPLY_DAMROLL;
    affect_to_char(victim, &af);
    af.location  = APPLY_STATS;
    affect_to_char(victim, &af);
    af.location  = APPLY_SAVES;
    af.modifier  = 0 - level / 10;
    affect_to_char( victim, &af );
    
    send_to_char( "You feel your god's energy surge through you.\n\r", victim );
    
    return;
}

void spell_deaths_door( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_DEATHS_DOOR))
    {
        send_to_char( "You already have the gods watching over you!\n\r", ch );
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DEATHS_DOOR;
    affect_to_char( victim, &af );
    send_to_char( "You have the gods watching over you!\n\r", victim );
    act("$n has the gods watching over $m.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_mana_heal( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->mana = UMIN( victim->mana + level, victim->max_mana );
    update_pos( victim );
    send_to_char( "You are surrounded by a blue glow, and your mind tingles.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_blessed_darkness( int sn, int level, CHAR_DATA *ch, void *vo, int target)   
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    
    
    /* character target */
    victim = (CHAR_DATA *) vo;
    
    if ( (time_info.hour > 5) && (time_info.hour < 20))
    {
        send_to_char("Darkness has not overtaken the land, and therefore cannot aid anyone.\n\r", ch); 
        return;
    }    
    
    if ( IS_AFFECTED( victim, AFF_DARKNESS ) )
    {
        if (victim == ch)
        { 
            send_to_char("You have already been instilled by the forces of the night.\n\r",ch);
            return;
        }
        else
        {
            act("$N already is energized by the night.",ch,NULL,victim,TO_CHAR);
            return;
        }
        
    }        

    if (IS_GOOD(victim))
    {
        send_to_char("The darkness refuses to meld with the good.\n\r",ch);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_DAMROLL;
    af.modifier  = level / 6; 
    af.bitvector = AFF_DARKNESS;
    affect_to_char( victim, &af );
    
    af.location  = APPLY_SAVES;
    af.modifier  = 0 - level / 6;
    affect_to_char( victim, &af );
    
    send_to_char( "You feel the darkness flow through you.\n\r", victim );
    if ( ch != victim )
        act("You call the darkness to instill $N with power.",ch,NULL,victim,TO_CHAR);
    return;  
}

void spell_glyph_of_evil(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam, curr_dam;
    int blessed_darkness_num, curse_num, frenzy_num;
    bool is_dark;
    
    blessed_darkness_num = skill_lookup("blessed darkness");
    curse_num = skill_lookup("curse");
    frenzy_num = skill_lookup("frenzy");
    
    act("$n utters a word of demonic power!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You utter a word of demonic power.\n\r",ch);
    
    if ( !IS_EVIL(ch) )
    {
	send_to_char("Evil will not assist you!\n\r",ch);
	return;
    }

    dam = get_sn_damage(sn, level, ch) * AREA_SPELL_FACTOR;

    if ( (time_info.hour > 5) && (time_info.hour < 20) )
	is_dark = FALSE;
    else
	is_dark = TRUE;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        
	if ( is_same_group(ch, vch) )
        {
	    /* little spellup */
	    if ( is_dark && !IS_AFFECTED(vch, AFF_DARKNESS) )
		spell_blessed_darkness(blessed_darkness_num,level,ch,(void *) vch,TARGET_CHAR);
	    if ( IS_EVIL(vch) && !IS_AFFECTED(vch, AFF_BERSERK) )
		spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR);
        }

	else if ( vch->fighting != NULL
		  && is_same_group(ch, vch->fighting)
		  && !is_safe_spell(ch,vch,TRUE)
		  && !IS_EVIL(vch) )
        {
	    if ( IS_GOOD(vch) && !IS_AFFECTED(vch, AFF_CURSE) )
		spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);

	    curr_dam = dam;
	    if ( saves_spell(vch, ch, level, DAM_NEGATIVE) )
		curr_dam /= 2;
	    full_dam(ch,vch,curr_dam,sn,DAM_NEGATIVE,TRUE);
        }
    }  
}

void spell_tomb_rot( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    /*
    if ( (victim->level - 10 > ch->level) || IS_IMMORTAL( victim ) )
    {
        send_to_char("Your target is too powerful to be rotted by your magic.\n\r", ch);
        return;
    }
    */
    
    if ( IS_AFFECTED(victim, AFF_TOMB_ROT) )
    {
        send_to_char("They are already rotting to death!\n\r",ch);
        return;
    }
    
    if ( saves_spell(victim, ch, level, DAM_DISEASE) )
    {
        if (ch == victim)
            send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
        else
            act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_VIT;
    af.modifier  = -level/2;
    af.bitvector = AFF_TOMB_ROT;
    affect_to_char(victim,&af);
    
    send_to_char("Your skin starts to dry and crackle.\n\r",victim);
    act("$n writhes in pain as $s skin starts to dry and crackle.", victim,NULL,NULL,TO_ROOM);
}


void spell_soreness( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( is_safe( ch, victim ) )
        return;
    
    if ( /*is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW) 
	   || IS_AFFECTED(victim, AFF_WEAKEN) ||*/ IS_AFFECTED(victim, AFF_SORE)) 
    {
        if (victim == ch)
            send_to_char("Don't you think you are hurting enough!\n\r",ch);
        else   
            
            act("$N is crippled already, show some mercy.",
            ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if ( saves_spell(victim, ch, level, DAM_OTHER) )
    {
        if (victim != ch)
            send_to_char("Nothing seemed to happen.\n\r",ch);
        
        send_to_char("You feel momentary ache in your muscles.\n\r",victim);
        return;
    }
    
    /*
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (!check_dispel(level,victim,skill_lookup("haste")))
        {
            if (victim != ch)
                send_to_char("Spell failed to have an effect.\n\r",ch);
            
            send_to_char("Your muscles ache for a moment.\n\r",victim);
            return;
        }
        
        act("$n is sore all of a sudden.",victim,NULL,NULL,TO_ROOM);
        return;
    }
    */
    
    af.where     = TO_AFFECTS;
    
    af.type      = sn;
    af.level     = level;  
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_AGI;
    af.modifier  = -1 - level/5;
    af.bitvector = AFF_SORE;
    affect_to_char( victim, &af );
    
    af.location  = APPLY_DEX;
    af.modifier  = -1 - level/5;
    affect_to_char( victim, &af );
    af.location  = APPLY_VIT;
    af.modifier  = -1 - level/5;
    
    affect_to_char( victim, &af );
    send_to_char( "You are hit with sudden aches!\n\r", victim );
    act("$n grimaces as if $e were suddenly sore.",victim,NULL,NULL,TO_ROOM);
    return;
}


/*Coded in by Korinn 1-15-99*/
void spell_mephistons_scrutiny(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    int detect_invis_num, detect_hidden_num, detect_astral_num, invis_num;
    
    detect_invis_num = skill_lookup("detect invis");
    detect_hidden_num = skill_lookup("detect hidden");
    detect_astral_num = skill_lookup("detect astral");
    invis_num = skill_lookup("invis");
    
    act("$n utters a praise to Lord Mephiston!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You praise the Vision of Mephiston.\n\r",ch);
    spell_detect_invis(detect_invis_num,level/6,ch,(void *) ch, TARGET_CHAR);
    spell_detect_hidden(detect_hidden_num,level/6,ch,(void *) ch, TARGET_CHAR);
    spell_detect_astral(detect_astral_num,level/6,ch,(void *) ch, TARGET_CHAR);
    spell_invis(invis_num,level/8,ch,(void *) ch, TARGET_CHAR);
}

void spell_rimbols_invocation(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam, main_dam = get_sn_damage( sn, level, ch ) * AREA_SPELL_FACTOR / 4;
    
    act("Rimbol channels the power of earth to form an avalanche!",ch,NULL,NULL,TO_ROOM);
    send_to_char("Rimbol answers your prayers by bringing forth Earth's power!\n\r",ch);
    
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	dam = main_dam;
        vch_next = vch->next_in_room;
        
        if ( !is_opponent(ch,vch) )
            continue;
        
        if ( saves_spell(vch, ch, level, DAM_BASH) )
	    dam /= 2;

	full_dam( ch, vch, dam, sn, DAM_BASH ,TRUE);
    }
    
    act("Rimbol summons the tornado's winds to slash down his foes!",ch,NULL,NULL,TO_ROOM);
    send_to_char("Your prayers are answered by Rimbol's powerful tornado!\n\r",ch);
    
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	dam = main_dam;
        vch_next = vch->next_in_room;
        
        if ( !is_opponent(ch,vch) )
            continue;
        
        if (IS_AFFECTED(vch,AFF_FLYING))
            dam = dam * 5/4;
        else
            dam = dam * 3/4;

        if ( saves_spell(vch, ch, level, DAM_SLASH) )
            dam /= 2;
        
        full_dam( ch, vch, dam, sn, DAM_SLASH ,TRUE);
    }
    
    act("The raging fire within Rimbol's soul incinerates his enemies!",ch,NULL,NULL,TO_ROOM);
    send_to_char("The raging fire within Rimbol's soul incinerates your enemies!\n\r",ch);
    
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	dam = main_dam;
        vch_next = vch->next_in_room;
        
        if ( !is_opponent(ch,vch) )
            continue;
        
        if ( saves_spell(vch, ch, level, DAM_FIRE) )
            dam /= 2;
        
        full_dam( ch, vch, dam, sn, DAM_FIRE ,TRUE);
    }
    
    act("The magical waters rise up and engulf those in its path!",ch,NULL,NULL,TO_ROOM);
    send_to_char("The waters rise up and engulf your enemies!\n\r",ch);
    
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	dam = main_dam;
        vch_next = vch->next_in_room;
        
        if ( !is_opponent(ch,vch) )
            continue;
        
        if ( saves_spell(vch, ch, level, DAM_DROWNING) )
            dam /= 2;
        
        full_dam( ch, vch, dam, sn, DAM_DROWNING ,TRUE);
    }
}


void spell_quirkys_insanity(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_ELEMENTAL_SHIELD))
    {
        if (victim == ch)
        {
            send_to_char("Your shield shimmers away as an aura of insanity protects you.\n\r",ch);
            affect_strip_flag(ch, AFF_ELEMENTAL_SHIELD);

            af.where     = TO_AFFECTS;
            af.type      = sn;
            af.level     = level;
            af.duration  = get_duration(sn, level);
            af.location  = APPLY_AC;
            af.modifier  = -level/8;
            af.bitvector = AFF_ELEMENTAL_SHIELD;
            affect_to_char( victim, &af );
            af.where = TO_RESIST;
            af.type = sn;
            af.level = level;
            af.duration = get_duration(sn, level);
            af.location = APPLY_NONE;
            af.modifier=0;
            af.bitvector=RES_MENTAL;
            affect_to_char(victim, &af);
    
            if (!IS_SET(victim->res_flags, RES_MENTAL))
            {
                af.where     = TO_AFFECTS;
                af.type      = sn;
                af.level     = level;
                af.duration  = get_duration(sn, level);
                af.location  = APPLY_AC;
                af.modifier  = -level/8;
                af.bitvector = AFF_ELEMENTAL_SHIELD;
                affect_to_char( victim, &af );
                af.where = TO_RESIST;
                af.type = sn;
                af.level = level;
                af.duration = get_duration(sn, level);
                af.location = APPLY_NONE;
                af.modifier=0;
                af.bitvector=RES_MENTAL;
                affect_to_char(victim, &af);
            }
        }
        else
            act("$N is already shielded.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_AC;
    af.modifier  = -level/8;
    af.bitvector = AFF_ELEMENTAL_SHIELD;
    affect_to_char( victim, &af );
    af.where = TO_RESIST;
    af.type = sn;
    af.level = level;
    af.duration = get_duration(sn, level);
    af.location = APPLY_NONE;
    af.modifier=0;
    af.bitvector=RES_MENTAL;
    affect_to_char(victim, &af);
    send_to_char( "Quirky's insanity runs through your mind.\n\r", victim );
    if ( ch != victim )
        act("Quirky's insanity takes over $N's mind.",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_sivas_sacrifice( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int harm;
    bool half_dam = FALSE;
    
    /* reduce effect if opponent is sanced */
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	half_dam = TRUE;

    ch->mana += skill_table[sn].min_mana;
    harm = ch->mana;
    harm = UMIN( harm, victim->hit - victim->max_hit/8 );
    harm = UMAX( harm, 0 );

    /* reduce effect if opponent is sanced */
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
	harm = UMIN( 2*harm, ch->mana );
	ch->mana -= harm;
	harm /= 2;
    }
    else
	ch->mana -= harm;

    victim->hit-=harm;
    remember_attack(victim,ch,harm);
    dam_message(ch,victim,harm,sn, FALSE);
    
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    
    return;
}


void spell_smotes_anachronism( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
   	CHAR_DATA *gch;
   	CHAR_DATA *victim;
    
    act("Smote appears suddenly and slows down time!",ch,NULL,NULL,TO_ROOM);
    send_to_char("Your prayers are answered as Smote slows down time!\n\r",ch);
    ch->wait += PULSE_VIOLENCE;
    
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
        
    {
        if ( !is_same_group( gch, ch )
	     || was_obj_cast && gch != ch )
            continue;
        
        gch->wait = UMAX(gch->wait - PULSE_VIOLENCE, 0);
        gch->daze = UMAX(gch->daze - PULSE_VIOLENCE, 0);
        victim = gch->fighting;
        
	if ( gch->hit  < gch->max_hit )
	    gch->hit  += hit_gain(gch) / 10;
	else
	    gch->hit = gch->max_hit;
        
	if ( gch->mana < gch->max_mana )
	    gch->mana += mana_gain(gch) / 10;
	else
	    gch->mana = gch->max_mana;
	
	if ( gch->move < gch->max_move )
	    gch->move += move_gain(gch) / 10;
	else
	    gch->move = gch->max_move;	
        
        if ( ( victim = gch->fighting ) == NULL || ch->in_room == NULL )
            continue;
        
        if ( IS_AWAKE(gch) && gch->in_room == victim->in_room )
            multi_hit( gch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( gch, FALSE );
    }
}


void spell_prayer(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    AFFECT_DATA af;
    
    if (is_affected(ch, gsn_prayer) || is_affected(ch, gsn_bless))
    {
        send_to_char("You are already blessed.\n\r",ch);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 4;
    af.bitvector = 0;
    affect_to_char(ch, &af);
    
    af.location  = APPLY_SAVES;
    af.modifier  = -level/4;
    affect_to_char(ch, &af);
    
    af.location  = APPLY_AC;
    af.modifier  = -level/2;
    affect_to_char(ch, &af);
    
    af.location  = APPLY_WIS;
    af.modifier  = 6+level/10;
    affect_to_char(ch, &af);
    
    send_to_char( "You feel righteous.\n\r", ch);
    return;
}

void spell_breath_of_god(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = ch;
    bool found = FALSE;
    int sn1;
    
    for (sn1 = 1; skill_table[sn1].name != NULL; sn1++)
	if ( IS_SPELL(sn1)
	     && is_offensive(sn1)
	     && check_dispel(level * 3/4, victim, sn1) )
	    found = TRUE;

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}


void spell_stop(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    
    if ( saves_spell(victim, ch, level, DAM_MENTAL) )
    {
        if (victim != ch)
            send_to_char("Nothing seemed to happen.\n\r",ch);
        send_to_char("You feel momentarily lethargic.\n\r",victim);
        return;
    }
    
    /*
    if ( IS_AFFECTED(victim,AFF_HASTE) )
    {
        if (!check_dispel(level,victim,skill_lookup("haste")))
        {
            if (victim != ch)
                send_to_char("Spell failed to stop.\n\r",ch);
            send_to_char("You feel momentarily slower.\n\r",victim);
            return;
        }
        
        act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
        return;
    }
    */
    
    send_to_char( "You stop in your tracks!\n\r", victim );
    act("$n stands perfectly still.",victim,NULL,NULL,TO_ROOM);
    victim->stop += 1;
    victim->wait += PULSE_VIOLENCE;
    return;
}

void spell_mana_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if (is_affected(victim, skill_lookup("mana shield")))
    {
        if (victim == ch)
            send_to_char("You are already shielded.\n\r",ch);
        else
            act("$N is already shielded.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel your mana protecting you.\n\r", victim );
    act( "$n is protected by $s mana.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_mantra( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_MANTRA) )
    {
        if (victim == ch)
            send_to_char("You are already in touch with your mantra.\n\r",ch);
        else
            act("$N is already in touch with $s mantra.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_MANTRA;
    affect_to_char( victim, &af );
    send_to_char( "You feel in touch with your mantra.\n\r", victim );
    act( "$n gets in touch with $s mantra.", victim, NULL, NULL, TO_ROOM );
    return;
}

/* Miscellaneous stuff */

void spell_decompose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    AFFECT_DATA af;
    
    if ( is_affected(victim, sn) )
    {
	send_to_char( "Your victim is already decomposing!\n\r", ch );
	return;
    }
    
    if ( saves_spell(victim, ch, level, DAM_HARM) || saves_physical(victim, NULL, level, DAM_HARM) )
    {
	send_to_char( "A wave of malvolent energy passes over your body.\n\r", victim );
	send_to_char( "Spell failed to start decomposing.\n\r", ch );
	return;
    }
    
    /* good, we hit 'em :) */
    send_to_char("You feel an intense pain in your body.\n\r",victim);
    act("$n jerks in sudden pain.",victim,0,0,TO_ROOM);

    af.where = TO_AFFECTS;
    af.level = level;
    af.duration = get_duration(sn, level);
    af.type = sn;
    af.bitvector = 0;
    af.modifier = -1;

    /* start out with -1 on all 4 stats */
    /* affects will get worse over time, handled in special_affect_update */
    af.location = APPLY_STR;
    affect_to_char(victim,&af);
    af.location = APPLY_AGI;
    affect_to_char(victim,&af);
    af.location = APPLY_DEX;
    affect_to_char(victim,&af);
    af.location = APPLY_INT;
    affect_to_char(victim,&af);

    /* a bit damage won't harm anyone ;) */
    dam = get_sn_damage( sn, level, ch );
    full_dam( ch, victim, dam, sn, DAM_HARM, TRUE );
}

void spell_heal_mind( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    int diff;        /* diff between char max_mana and current mana */
    int price;       /* How Much will it cost */
    int afford;      /* If they can't do it all, how much can they afford */
    int factor = 5;  /* how many moves for 1 mana */

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_WAR) )
    {
	send_to_char("Your mind won't heal during the war.\n\r", ch);
	return;
    }

    /* restore spell cost */
    ch->mana += skill_table[sn].min_mana;
    
    if( ch->mana >= ch->max_mana )
    {
	send_to_char("Your mind is already healed.\n\r", ch);
	return;
    }

    diff = ch->max_mana - ch->mana;
    price = factor * diff;
    
    if ( (ch->move - 100) >= price )
    {
	send_to_char("You levitate and feel fine.\n\r", ch);
	ch->mana = ch->max_mana;
	ch->move -= price;
	return;
    }
	
    afford = (ch->move - 100) / factor;
    price = afford * factor;

    if ( afford < 1 )
    {
	send_to_char("You feel too tired for that.\n\r", ch);
	return;
    }

    ch->mana += afford;
    ch->move -= price;
    
    send_to_char("You levitate from the ground and feel better.\n\r", ch);
    return;
}

void spell_life_force ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int loss = UMIN(ch->hit * 9/10, victim->max_hit - victim->hit );

    ch->hit -= loss; 
    victim->hit += loss;     
    update_pos( victim );

    send_to_char("You feel alive again.\n\r", victim );
    if ( ch != victim )
      act( "You transfer your life force to $N.", ch, NULL, victim, TO_CHAR );
    return;
}

void spell_heal_all( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *gch;

    /* special: if cast via object, it heals the caster */
    if ( was_obj_cast )
    {
	if ( vo == NULL )
	    gch = ch;
	else
	    gch = (CHAR_DATA*)vo;

	act( "$n is heavily healed.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You feel outstanding!!!\n\r", gch );
	gch->hit = UMIN( gch->hit + 10*level, gch->max_hit );
	gch->mana = UMIN( gch->mana + 10*level, gch->max_mana );
	gch->move = UMIN( gch->move + 10*level, gch->max_move );
	/*
	if (gch->hit < gch->max_hit)   gch->hit  = gch->max_hit;
	if (gch->mana < gch->max_mana) gch->mana = gch->max_mana;
	if (gch->move < gch->max_move) gch->move = gch->max_move;
	*/
	update_pos( gch );
	return;
    }

    send_to_char( "You send out bursts of healing energy.\n\r", ch );
    act( "$n sends out bursts of healing energy.", ch, NULL, NULL, TO_ROOM );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( gch != ch )
	{
	    act( "$n is heavily healed.", gch, NULL, NULL, TO_ROOM );
	    send_to_char( "You feel outstanding!!!\n\r", gch );
	    gch->hit = UMIN( gch->hit + 10*level, gch->max_hit );
	    gch->mana = UMIN( gch->mana + 10*level, gch->max_mana );
	    gch->move = UMIN( gch->move + 10*level, gch->max_move );
	    /*
	    if (gch->hit < gch->max_hit)   gch->hit  = gch->max_hit;
	    if (gch->mana < gch->max_mana) gch->mana = gch->max_mana;
	    if (gch->move < gch->max_move) gch->move = gch->max_move;
	    */
	    update_pos( gch );
        }
    }
    return;
}

void spell_extinguish(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *light = (OBJ_DATA *) vo;

    if ( target == TARGET_CHAR )
    {
	light = get_eq_char( victim, WEAR_LIGHT );
	if ( light == NULL )
	{
	    if ( victim == ch )
		act( "You're not using any light source!", ch, NULL, NULL, TO_CHAR );
	    else
		act( "$N is not using any light source!", ch, NULL, victim, TO_CHAR );
	    return;
	}
    }

    if (light->item_type == ITEM_LIGHT)
    {
	if ( light->value[2] == 1 )
	{
	    act( "The light from $p is already fading!", ch, light, NULL, TO_CHAR );
	    return;
	}
	/* check for infinite light sources */
	if ( light->value[2] < 1
	     || number_bits(1)
	     || number_range(0, level) <= number_range(0, light->level) )
	{
	    act( "The light from $p shines too bright!", ch, light, NULL, TO_CHAR );
	    return;
	}
	/* set light to go out soon */
	light->value[2] = 1;
	act( "The light from $p starts to fade.",ch,light,NULL,TO_ALL);
	return;
    }

    if (IS_OBJ_STAT(light, ITEM_GLOW))
    {
	REMOVE_BIT(light->extra_flags, ITEM_GLOW);
	act("$p loses its glow.",ch,light,NULL,TO_ALL);
	return;
    }
    
    if (IS_OBJ_STAT(light, ITEM_DARK))
    {
	act("$p is already dark as the night.", ch, light, NULL, TO_CHAR);
	return;
    }

    SET_BIT(light->extra_flags, ITEM_DARK);
    act("A dark aura encompasses $p.", ch, light, NULL, TO_ALL); 
}

void spell_renewal( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA *aff;
    int cost, type, last_type = 0, base_duration, max_duration;
    bool last_renew, last_paid = FALSE, found = FALSE;

    for ( aff = ch->affected; aff != NULL; aff = aff->next )
    {
        type = aff->type;

        if ( type != last_type )
        {
            if ( !IS_SPELL(type) || is_offensive(type) || aff->duration == -1
                || (base_duration = get_duration(type, level)) == 0 )
            {
                last_renew = FALSE;
                continue;
            }

            found = TRUE;
            last_type = type;
            last_renew = TRUE;
            last_paid = FALSE;
            // get max duration once for each spell
            max_duration = number_range(base_duration, 2 * base_duration);            
        }
        else if ( !last_renew )
            continue;

        if ( aff->duration >= max_duration )
            continue;

        /* renewing a spell to full duration costs twice as much as casting it fresh */
        if ( !last_paid )
        {
            cost = 2 * meta_magic_adjust_cost(ch, mastery_adjust_cost(skill_table[type].min_mana, get_mastery(ch, sn)), FALSE);
            cost = (cost + base_duration - 1) / base_duration; // round up
            if ( ch->mana < cost )
            {
                printf_to_char( ch, "You don't have enough mana to renew your %s spell.\n\r", skill_table[type].name );
                last_renew = FALSE;
                continue;
            }
            ch->mana -= cost;
            last_paid = TRUE;

            printf_to_char( ch, "Your %s spell has been renewed.\n\r", skill_table[type].name );
        }
        
        aff->duration += 1;
        if ( aff->level < level )
            aff->level += 1;
    }

    if ( !found )
        send_to_char( "There are no spells on you to renew.\n\r", ch );
}

void spell_reflection( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch, AFF_REFLECTION) )
    {
	send_to_char( "You are already reflecting spells.\n\r", ch );
	return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_REFLECTION;
    af.where     = TO_AFFECTS;
    affect_to_char( ch, &af );

    act( "You are surrounded by a reflecting aura.", ch, NULL, NULL, TO_CHAR );
    act( "$n is surrounded by a reflecting aura.", ch, NULL, NULL, TO_ROOM );
}


void spell_mimic( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA*)vo;
    MOB_INDEX_DATA *mimic_mob;

    if ( IS_NPC(ch) )
	return;

    if ( ch == victim )
    {
	send_to_char( "No need to mimic yourself..\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && !is_mimic(victim) )
    {
	send_to_char( "You can only mimic NPCs.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
	mimic_mob = victim->pIndexData;
    else
	mimic_mob = get_mimic(victim);

    if ( mimic_mob == NULL || is_empty_string(mimic_mob->short_descr) 
	|| IS_SET(mimic_mob->act, ACT_NOMIMIC) || IS_SET(mimic_mob->act, ACT_IS_CHANGER) )
    {
	send_to_char( "That won't work.\n\r", ch );
	return;
    }

    affect_strip( ch, sn );

    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.where     = TO_SPECIAL;
    af.bitvector = mimic_mob->vnum;

    affect_to_char( ch, &af );

    act( "You assume the appearance of $N.", ch, NULL, victim, TO_CHAR );
}

void spell_mirror_image( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;

    affect_strip( ch, sn );
    affect_strip( ch, gsn_phantasmal_image );

    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.modifier  = -10;
    af.location  = APPLY_AC;
    af.where     = TO_SPECIAL;
    af.bitvector = dice(2,4) + level/16; // number of images

    affect_to_char( ch, &af );

    act( "You are surrounded with images of yourself.", ch, NULL, NULL, TO_CHAR );
    act( "$n is surrounded with images of $mself.", ch, NULL, NULL, TO_ROOM );
}

void spell_haunt(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_HAUNTED) )
    {
	send_to_char( "They are already haunted.\n\r", ch );
	return;
    }
    
    if ( saves_spell(victim, ch, level, DAM_OTHER) )
    {
        if (victim != ch)
	    send_to_char("The spirits don't answer your call.\n\r",ch);
        send_to_char("A cold shiver runs down your spine.\n\r",victim);
        return;
    }
    
    send_to_char( "Spirits are coming to haunt you!\n\r", victim );
    act( "The spirits start to haunt $n.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_LUC;
    af.modifier  = -level/4;
    af.bitvector = AFF_HAUNTED;
    affect_to_char( victim, &af );

    return;
}

void renew_affect( CHAR_DATA *ch, int sn, int level )
{
    AFFECT_DATA *aff;

    for ( aff = ch->affected; aff != NULL; aff = aff->next )
    {
	if ( aff->type != sn )
	    continue;

	if ( aff->duration >= 0 )
	    aff->duration++;

	if ( aff->level < level )
	    aff->level++;
    }
}

void spell_dancing_bones( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *gch = (CHAR_DATA *) vo;
    int heal, gsn_anim, gsn_ghost;
    
    gsn_anim = skill_lookup( "animate dead" );
    gsn_ghost = skill_lookup( "ghost chant" );
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( !is_same_group(gch, ch) || !NPC_ACT(gch, ACT_UNDEAD) )
            continue;

        heal = get_sn_heal(sn, level, ch, gch);
        gch->hit = UMIN( gch->hit + heal, gch->max_hit );
        update_pos( gch );

        /* prolong the animate effect on zombies & ghosts */
        renew_affect( gch, gsn_anim, level );
        renew_affect( gch, gsn_ghost, level );
    }
    send_to_char( "The bones of your undead followers mend.\n\r", ch );
}

void spell_mana_burn( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_MANA_BURN) )
    {
	send_to_char( "Their mana is already boiling.\n\r", ch );
	return;
    }
    
    if ( saves_spell(victim, ch, level, DAM_OTHER) )
    {
        if (victim != ch)
	    send_to_char( "Their mana remains cool.\n\r",ch);
	send_to_char("A warm shiver runs down your spine.\n\r",victim);
        return;
    }
    
    send_to_char( "Your mana starts burning!\n\r", victim );
    act( "$n's mana starts to burn.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_WIS;
    af.modifier  = -level/4;
    af.bitvector = AFF_MANA_BURN;
    affect_to_char( victim, &af );
}

void spell_iron_maiden( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_IRON_MAIDEN) )
    {
	send_to_char( "They are already being tortured.\n\r", ch );
	return;
    }
    
    if ( saves_spell(victim, ch, level, DAM_MENTAL) )
    {
        if (victim != ch)
	    send_to_char( "They resist your torturing attempts.\n\r",ch);
	send_to_char("Something seems to sting you briefly.\n\r",victim);
        return;
    }
    
    send_to_char( "You're being tortured with 1000 needles!\n\r", victim );
    act( "$n is being tortured by 1000 needles!", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_IRON_MAIDEN;
    affect_to_char( victim, &af );
}

/* New good weather spell added by Astark & fixed by Bobble */
void spell_solar_flare( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
    if ( weather_info.sky >= SKY_RAINING || !room_is_sunlit(ch->in_room) )
    {
        send_to_char( "There isn't enough sunshine out for that!\n\r", ch );
        return FALSE;
    }
    
    /* the better the weather, and the brighter the day more powerful */
    dam = get_sn_damage( sn, level, ch ) / (2 + weather_info.sky + (weather_info.sunlight == SUN_LIGHT ? 0 : 1));
    
    // some fire damage ...
    act( "You call upon the heat of the sun to sear $N's flesh!", ch, NULL, victim, TO_CHAR);
    act( "$n calls upon the heat of the sun to sear your flesh!", ch, NULL, victim, TO_VICT);
    act( "$n calls upon the heat of the sun to sear $N's flesh!", ch, NULL, victim, TO_NOTVICT);
    
    if ( !saves_spell(victim, ch, level, DAM_FIRE) )
        full_dam(ch, victim, dam, sn, DAM_FIRE, TRUE);
    else
        full_dam(ch, victim, dam/2, sn, DAM_FIRE, TRUE);

    CHECK_RETURN(ch, victim);

    // ... and some light damage & blindness
    act( "You call upon the light of the sun to blind $N!", ch, NULL, victim, TO_CHAR);
    act( "$n calls upon the light of the sun to blind you!", ch, NULL, victim, TO_VICT);
    act( "$n calls upon the light of the sun to blind $N!", ch, NULL, victim, TO_NOTVICT);
    
    if ( !saves_spell(victim, ch, level, DAM_LIGHT) )
    {
        full_dam(ch, victim, dam, sn, DAM_LIGHT, TRUE);
        CHECK_RETURN(ch, victim);
        // chance to blind
        spell_blindness( gsn_blindness, level / 2, ch, (void *)victim, TARGET_CHAR );
    }
    else
        full_dam(ch, victim, dam/2, sn, DAM_LIGHT, TRUE);

    return;
}


/* Overcharge. Added by Astark. SirLance's Idea */

void spell_overcharge( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    //CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( is_affected(ch, gsn_overcharge))
    {
        send_to_char( "You're already overcharged!\n\r", ch );
        return;
    }
    //printf_to_char(ch," gsn: %d sn: %d\n\r", gsn_overcharge, sn);
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_OVERCHARGE;
//    af.bitvector = 0;
    affect_to_char( ch, &af );
    send_to_char( "You focus intensely as your mana begins to overcharge!\n\r", ch );
    act("$n begins to focus as $s mana starts to overcharge",ch,NULL,NULL,TO_ROOM);
    return;
}


/* Unearth. Added by Astark. Mage/Illu/Cleric spell. Small chance to destance opponent */

void spell_unearth( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
        act("$N hovers safely above the shaking ground.", ch, NULL, victim, TO_CHAR);
        return;
    }

    act("You cause the earth beneath $N to break and shift violently.", ch, NULL, victim, TO_CHAR);
    act("$n causes the earth beneath you to break and shift violently.", ch, NULL, victim, TO_VICT);
    act("$n causes the earth beneath $N to break and shift violently.", ch, NULL, victim, TO_NOTVICT);
    
    int dam = get_sn_damage(sn, level, ch);

    if ( saves_spell(victim, ch, level, DAM_BASH) )
        dam /= 2;
    else
    {
        if ( victim->position > POS_RESTING )
        {
            act("The rippling earth causes you to fall.", victim, NULL, NULL, TO_CHAR);
            act("The rippling earth causes $n to fall.", victim, NULL, NULL, TO_ROOM);
            set_pos(victim, POS_RESTING);
        }
        destance(victim, get_mastery(ch, sn));
    }
    full_dam(ch, victim, dam, sn, DAM_BASH, TRUE);
}

void spell_shadow_shroud(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( is_affected(ch, gsn_shadow_shroud))
    {
        send_to_char( "You're already shrouded in darkness!\n\r", ch );
        return;
    }

            af.where     = TO_AFFECTS;
            af.type      = sn;
            af.level     = level;
            af.duration  = get_duration(sn, level);
            af.modifier  = level / 6;
            af.bitvector = 0;
            af.location  = APPLY_DEX;
            affect_to_char(victim,&af);
            af.location  = APPLY_AGI;
            affect_to_char(victim,&af);
            af.location  = APPLY_LUC;
            affect_to_char(victim,&af);

       
    send_to_char( "You are surrounded by darkness.\n\r", victim );
    if ( ch != victim )
        act("$N is surrounded by an aura of darkness.",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_astarks_rejuvenation( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *gch;
    bool found = FALSE;
    int heal;
    int refr;
    int sn1;    

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( !is_same_group( gch, ch ) )
            continue;

        heal = get_sn_heal( sn, level, ch, gch ) * 6/15;
        gch->hit = UMIN( gch->hit + heal, gch->max_hit );
        
        refr = get_sn_heal( sn, level, ch, gch ) * 4/15;
        gch->move = UMIN( gch->move + refr, gch->max_move );

        update_pos( gch );

        send_to_char( "You feel much better!\n\r", gch );
        check_sn_multiplay( ch, gch, sn );

        for (sn1 = 1; skill_table[sn1].name != NULL; sn1++)
        {
            if (IS_SPELL(sn1)
               && is_offensive(sn1)
               && sn1 != gsn_charm_person
               && check_dispel(level/2, gch, sn1) )
            found = TRUE;
        }

    }
    send_to_char( "Astark reaches down from the heavens and rejuvenates your allies.\n\r", ch );
    return;
}


void spell_phase(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( is_affected(ch, gsn_phase))
    {
        send_to_char("You are already phasing away from spells.\n\r",ch);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PHASE;
    affect_to_char( victim, &af );
    send_to_char( "You begin to phase in and out of existence.\n\r", victim );
    act("$n begins to phase in and out of existence.", victim, NULL, NULL, TO_ROOM);
}


void spell_conviction (int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char buf[MSL];

    if (victim == ch)
    {
        send_to_char("How much conviction can one really have against themselves?\n\r", ch);
        return;
    }

    if (IS_NEUTRAL(ch))
    {
        send_to_char("Without firm beliefs you can't have much conviction.\n\r", ch);
        return;
    }

    // same-aligned targets are safe
    if ((IS_GOOD(ch) && IS_GOOD(victim)) || (IS_EVIL(ch) && IS_EVIL(victim))) 
    {
        act_new( "$N's beliefs do not conflict with yours.", ch, NULL, victim, TO_CHAR, POS_RESTING);  
        return;
    }
    
    // opposite aligned targets get hurt
    int align_diff = abs(ch->alignment - victim->alignment);
    int dam = get_sn_damage( sn, level, ch, victim );
    if (IS_GOOD(ch))
        dam = dam * align_diff / 1000;
    else 
        dam = dam * align_diff / 1350;
    
    if ( saves_spell(victim, ch, level, DAM_MENTAL) )
        dam /= 2;
    
    full_dam(ch, victim, dam, sn, DAM_MENTAL, TRUE);
    
    return;
}


void spell_basic_apparition( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    CHAR_DATA *mob;
    CHAR_DATA *check;
    char buf[MAX_STRING_LENGTH];
    int mlevel, mhp, chance;
    int charmed, max;
     
    if (IS_SET(ch->act, PLR_WAR))
    {
        send_to_char("Apparitions have no place in wars like these.\n\r", ch );
        return;
    }

    if ( ch->pet != NULL )
    {
	send_to_char("You already control a pet.\n\r",ch);
	return;
    }
    
    chance = (get_curr_stat(ch, STAT_LUC)) / 2;
    
    if ( number_percent() > chance ) 
    {
        send_to_char( "You fail to summon an apparition.\n\r", ch);
        return;
    }
    
    if ((mob = create_mobile(get_mob_index(MOB_VNUM_BASIC_APPARITION)))==NULL) 
        return;

    send_to_char( "You summon a ghostly apparition to help you out!\n\r", ch );
    act( "$n summons a ghostly apparation.", ch, NULL, NULL, TO_ROOM );
    
    mlevel = URANGE(1, level * 3/4, ch->level);
    set_mob_level( mob, mlevel );

    sprintf(buf,"%s\n\rThis apparition belongs to %s.\n\r\n\r",
        mob->description,ch->name);
    free_string(mob->description);
    mob->description = str_dup(buf);
    
    char_to_room( mob, ch->in_room );
    
    add_follower( mob, ch );
    mob->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );
    SET_BIT(mob->act, ACT_PET);
    ch->pet = mob;
    
    return;
}


void spell_holy_apparition( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    CHAR_DATA *mob;
    CHAR_DATA *check;
    char buf[MAX_STRING_LENGTH];
    int mlevel, chance;
    int charmed, max;
      
    if (IS_SET(ch->act, PLR_WAR))
    {
        send_to_char("Apparitions have no place in wars like these.\n\r", ch );
        return;
    }

    if ( ch->pet != NULL )
    {
	send_to_char("You already control a pet.\n\r",ch);
	return;
    }

    if (!IS_GOOD(ch))
    {
	send_to_char("You aren't holy enough for that.\n\r",ch);
        return;
    }
    
    chance = (get_curr_stat(ch, STAT_LUC)) / 2;
    
    if ( number_percent() > chance ) 
    {
        send_to_char( "You fail to summon an apparition.\n\r", ch);
        return;
    }
    
    if ((mob = create_mobile(get_mob_index(MOB_VNUM_HOLY_APPARITION)))==NULL) 
        return;

    send_to_char( "You summon a holy apparition to help you out!\n\r", ch );
    act( "$n summons a holy apparation.", ch, NULL, NULL, TO_ROOM );

    mlevel = URANGE(1, level * 4/5, ch->level);
    set_mob_level( mob, mlevel );

    sprintf(buf,"%s\n\rThis apparition belongs to %s.\n\r\n\r",
        mob->description,ch->name);
    free_string(mob->description);
    mob->description = str_dup(buf);
    
    char_to_room( mob, ch->in_room );
    
    add_follower( mob, ch );
    mob->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );
    SET_BIT(mob->act, ACT_PET);
    ch->pet = mob;
    
    return;
}



void spell_phantasmal_image( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;

    affect_strip( ch, sn );
    affect_strip( ch, gsn_mirror_image );

    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.modifier  = -10;
    af.location  = APPLY_AC;
    af.where     = TO_SPECIAL;
    af.bitvector = dice(2,4) + level/8; // number of images

    affect_to_char( ch, &af );

    act( "You are surrounded with phantasmal images of yourself.", ch, NULL, NULL, TO_CHAR );
    act( "$n is surrounded with phantasmal images of $mself.", ch, NULL, NULL, TO_ROOM );
}



void spell_shroud_of_darkness( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(victim, AFF_SHROUD) )
    {
        if (victim == ch)
            send_to_char("You are already enclosed in darkness.\n\r",ch);
        else
            act("$N is already enclosed in darkness.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_SAVES;
    af.modifier  = -(level / 10);
    af.bitvector = AFF_SHROUD;
    affect_to_char( victim, &af );
    af.where     = TO_RESIST;
    af.location  = APPLY_AC;
    af.modifier  = -level;
    af.bitvector = RES_LIGHT;
    affect_to_char( victim, &af );
    act( "$n is encased in darkness.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are encased in darkness.\n\r", victim );
    return;
}

void spell_paralysis_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED( victim, AFF_PARALYSIS) )
    {
        act("$E is already barely able to move.", ch, NULL, victim, TO_CHAR);
        return;
    }
    
    if ( saves_spell(victim, ch, level, DAM_POISON) )
    {
        act("$N's body resists the poison!",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    af.type      = sn;
    af.where     = TO_AFFECTS;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_AGI;
    af.modifier  = -1 * (level / 14);
    af.bitvector = AFF_PARALYSIS;
    affect_to_char( victim, &af );
    send_to_char( "Your limbs feel heavy and weak.\n\r", victim );
    act("$n is having trouble moving.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_hallow(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( victim == ch )
    {
        send_to_char( "Hallow is reserved for helping your allies.\n\r", ch );
        return;
    }
    
    if (IS_AFFECTED(victim,AFF_HALLOW))
    {
        send_to_char("They are already hallowed.\n\r",ch);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_INT;
    af.modifier  = 16+ (level / 12);
    af.bitvector = AFF_HALLOW;
    affect_to_char(victim, &af);
    

    act( "You hallow $N.", ch, NULL, victim, TO_CHAR );
    act( "$N hallows you.", victim, NULL, ch, TO_CHAR );

    return;
}


void spell_minor_fade(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED(ch, AFF_FADE) || IS_AFFECTED(ch, AFF_MINOR_FADE))
    {
        send_to_char("You are already fading out of existence.\n\r",ch);
        return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = get_duration(sn, level);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_MINOR_FADE;
    affect_to_char( victim, &af );
    send_to_char( "Your body partially fades in and out of existence.\n\r", victim );
    act("$n begins to partially fade in and out of existence.", victim, NULL, NULL, TO_ROOM);
}


void spell_replenish( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    AFFECT_DATA af;

    if (IS_AFFECTED(ch,AFF_REPLENISH) || is_affected(ch, gsn_replenish_cooldown))
    {
        send_to_char("You replenished too recently to do that again.\n\r",ch);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_REPLENISH;
    affect_to_char( ch, &af );
       
    af.where     = TO_AFFECTS;
    af.type      = gsn_replenish_cooldown;
    af.duration  = 10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char(ch,&af);

    send_to_char( "The air all around you starts to replenish your health.\n\r", ch );
    act("$n's wounds begin to mend as the air in the room replenishes $s body.", ch, NULL, NULL, TO_ROOM);
}
 
