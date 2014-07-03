#include <lualib.h>
#include <lauxlib.h>
#include <time.h>
#include "merc.h"
#include "timer.h"
#include "lua_main.h"
#include "lua_arclib.h"
#include "interp.h"
#include "mudconfig.h"


lua_State *g_mud_LS = NULL;  /* Lua state for entire MUD */
bool       g_LuaScriptInProgress=FALSE;
int        g_ScriptSecurity=0;
int        g_LoopCheckCounter;

#define LUA_LOOP_CHECK_MAX_CNT 10000 /* give 1000000 instructions */
#define LUA_LOOP_CHECK_INCREMENT 100
#define ERR_INF_LOOP      -1

int GetLuaMemoryUsage()
{
    return lua_gc( g_mud_LS, LUA_GCCOUNT, 0);
}

int GetLuaGameObjectCount()
{
    lua_getglobal( g_mud_LS, "UdCnt");
    if (CallLuaWithTraceBack( g_mud_LS, 0, 1) )
    {
        bugf ( "Error with UdCnt:\n %s",
                lua_tostring(g_mud_LS, -1));
        return -1;
    }

    int rtn=luaL_checkinteger( g_mud_LS, -1 );
    lua_pop( g_mud_LS, 1 );
    return rtn;
}

int GetLuaEnvironmentCount()
{
    lua_getglobal( g_mud_LS, "EnvCnt");  
    if (CallLuaWithTraceBack( g_mud_LS, 0, 1) )
    {
        bugf ( "Error with EnvCnt:\n %s",
                lua_tostring(g_mud_LS, -1));
        return -1;
    }

    int rtn=luaL_checkinteger( g_mud_LS, -1 );
    lua_pop( g_mud_LS, 1 );
    return rtn;
}

static void infinite_loop_check_hook( lua_State *LS, lua_Debug *ar)
{
    if (!g_LuaScriptInProgress)
        return;

    if ( g_LoopCheckCounter < LUA_LOOP_CHECK_MAX_CNT)
    {
        g_LoopCheckCounter++;
        return;
    }
    else
    {
        /* exit */
        luaL_error( LS, "Interrupted infinite loop." );
        return;
    }
}



void stackDump (lua_State *LS) {
    int i;
    int top = lua_gettop(LS);
    for (i = 1; i <= top; i++) {  /* repeat for each level */
        int t = lua_type(LS, i);
        switch (t) {

            case LUA_TSTRING:  /* strings */
                logpf("`%s'", lua_tostring(LS, i));
                break;

            case LUA_TBOOLEAN:  /* booleans */
                logpf(lua_toboolean(LS, i) ? "true" : "false");
                break;

            case LUA_TNUMBER:  /* numbers */
                logpf("%g", lua_tonumber(LS, i));
                break;

            default:  /* other values */
                logpf("%s", lua_typename(LS, t));
                break;

        }
    }
    logpf("\n");  /* end the listing */
}

const char *check_string( lua_State *LS, int index, size_t size)
{
    size_t rtn_size;
    const char *rtn=luaL_checklstring( LS, index, &rtn_size );
    /* Check >= because we assume 'size' argument refers to
       size of a char buffer rather than desired strlen.
       If called with MSL then the result (including terminating '\0'
       will fit in MSL sized buffer. */
    if (rtn_size >= size )
        luaL_error( LS, "String size %d exceeds maximum %d.",
                (int)rtn_size, (int)size-1 );

    return rtn;
}

/* Run string.format using args beginning at index 
   Assumes top is the last argument*/
const char *check_fstring( lua_State *LS, int index, size_t size)
{
    int narg=lua_gettop(LS)-(index-1);

    if ( (narg>1))
    {
        lua_getglobal( LS, "string");
        lua_getfield( LS, -1, "format");
        /* kill string table */
        lua_remove( LS, -2);
        lua_insert( LS, index );
        lua_call( LS, narg, 1);
    }

    return check_string( LS, index, size);
}

static void GetTracebackFunction (lua_State *LS)
{
    lua_pushliteral (LS, LUA_DBLIBNAME);     /* "debug"   */
    lua_rawget      (LS, LUA_GLOBALSINDEX);    /* get debug library   */

    if (!lua_istable (LS, -1))
    {
        lua_pop (LS, 2);   /* pop result and debug table  */
        lua_pushnil (LS);
        return;
    }

    /* get debug.traceback  */
    lua_pushstring(LS, "traceback");
    lua_rawget    (LS, -2);               /* get traceback function  */

    if (!lua_isfunction (LS, -1))
    {
        lua_pop (LS, 2);   /* pop result and debug table  */
        lua_pushnil (LS);
        return;
    }

    lua_remove (LS, -2);   /* remove debug table, leave traceback function  */
}  /* end of GetTracebackFunction */

int CallLuaWithTraceBack (lua_State *LS, const int iArguments, const int iReturn)
{
    int error;

    int base = lua_gettop (LS) - iArguments;  /* function index */
    GetTracebackFunction (LS);
    if (lua_isnil (LS, -1))
    {
        lua_pop (LS, 1);   /* pop non-existent function  */
        bugf("pop non-existent function");
        error = lua_pcall (LS, iArguments, iReturn, 0);
    }
    else
    {
        lua_insert (LS, base);  /* put it under chunk and args */
        error = lua_pcall (LS, iArguments, iReturn, base);
        lua_remove (LS, base);  /* remove traceback function */
    }

    return error;
}  /* end of CallLuaWithTraceBack  */

void do_lboard( CHAR_DATA *ch, char *argument)
{
    lua_getglobal(g_mud_LS, "do_lboard");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        bugf ( "Error with do_lboard:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void do_lhistory( CHAR_DATA *ch, char *argument)
{
    lua_getglobal(g_mud_LS, "do_lhistory");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        bugf ( "Error with do_lhistory:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void update_lboard( int lboard_type, CHAR_DATA *ch, int current, int increment )
{
    if (IS_NPC(ch) || IS_IMMORTAL(ch) )
        return;

    lua_getglobal(g_mud_LS, "update_lboard");
    lua_pushnumber( g_mud_LS, lboard_type);
    lua_pushstring( g_mud_LS, ch->name);
    lua_pushnumber( g_mud_LS, current);
    lua_pushnumber( g_mud_LS, increment);

    if (CallLuaWithTraceBack( g_mud_LS, 4, 0) )
    {
        bugf ( "Error with update_lboard:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void save_lboards()
{
    lua_getglobal( g_mud_LS, "save_lboards");
    if (CallLuaWithTraceBack( g_mud_LS, 0, 0) )
    {
        bugf ( "Error with save_lboard:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }  
}

void load_lboards()
{
    lua_getglobal( g_mud_LS, "load_lboards");
    if (CallLuaWithTraceBack( g_mud_LS, 0, 0) )
    {
        bugf ( "Error with load_lboards:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void check_lboard_reset()
{
    lua_getglobal( g_mud_LS, "check_lboard_reset");
    if (CallLuaWithTraceBack( g_mud_LS, 0, 0) )
    {
        bugf ( "Error with check_lboard_resets:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

static int L_save_mudconfig(lua_State *LS)
{
    int i;
    CFG_DATA_ENTRY *en;

    lua_getglobal(LS, "SaveTbl");
    lua_newtable(LS);
    for ( i=0 ; mudconfig_table[i].name ; i++ )
    {
        en=&mudconfig_table[i];
        switch( en->type )
        {
            case CFG_INT:
            {
                lua_pushinteger( LS, *((int *)(en->value)));
                break;
            }
            case CFG_FLOAT:
            {
                lua_pushnumber( LS, *((float *)(en->value)));
                break;
            }
            case CFG_STRING:
            {
                lua_pushstring( LS, *((char **)(en->value)));
                break;
            }
            case CFG_BOOL:
            {
                lua_pushboolean( LS, *((bool *)(en->value)));
                break;
            }
            default:
            {
                luaL_error( LS, "Bad type.");
            }
        }

        lua_setfield(LS, -2, en->name);
    }

    return 1;
}

void save_mudconfig()
{
    lua_getglobal( g_mud_LS, "save_mudconfig");
    if (CallLuaWithTraceBack( g_mud_LS, 0, 0) )
    {
        bugf ( "Error with save_mudconfig:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void load_mudconfig()
{
    lua_getglobal( g_mud_LS, "load_mudconfig");
    if (CallLuaWithTraceBack( g_mud_LS, 0, 0) )
    {
        bugf ( "Error with load_mudconfig:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }    
}

bool run_lua_interpret( DESCRIPTOR_DATA *d)
{
    if (!d->lua.interpret) /* not in interpreter */
        return FALSE; 

    if (!strcmp( d->incomm, "@") )
    {
        /* kick out of interpret */
        d->lua.interpret=FALSE;
        d->lua.incmpl=FALSE;

        lua_unregister_desc(d);

        ptc(d->character, "Exited lua interpreter.\n\r");
        return TRUE;
    }

    lua_getglobal( g_mud_LS, "run_lua_interpret"); //what we'll call if no errors

    /* Check this all in C so we can exit interpreter for missing env */
    lua_pushlightuserdata( g_mud_LS, d); /* we'll check this against the table */
    lua_getglobal( g_mud_LS, INTERP_TABLE_NAME);
    if (lua_isnil( g_mud_LS, -1) )
    {
        bugf("Couldn't find " INTERP_TABLE_NAME);
        lua_settop(g_mud_LS, 0);
        return TRUE;
    }
    bool interpalive=FALSE;
    lua_pushnil( g_mud_LS);
    while (lua_next(g_mud_LS, -2) != 0)
    {
        lua_getfield( g_mud_LS, -1, "desc");
        if (lua_equal( g_mud_LS, -1,-5))
        {
           interpalive=TRUE;
        }
        lua_pop(g_mud_LS, 2);

        if (interpalive)
            break;
    }
    if (!interpalive)
    {
        ptc( d->character, "Interpreter session was closed, was object destroyed?\n\r"
                "Exiting interpreter.\n\r");
        d->lua.interpret=FALSE;

        ptc(d->character, "Exited lua interpreter.\n\r");
        lua_settop(g_mud_LS, 0);
        return TRUE;
    }
    /* object pointer should be sitting at -1, interptbl at -2, desc lightud at -3 */
    lua_remove( g_mud_LS, -3);
    lua_remove( g_mud_LS, -2);


    lua_getglobal( g_mud_LS, ENV_TABLE_NAME);
    if (lua_isnil( g_mud_LS, -1) )
    {
        bugf("Couldn't find " ENV_TABLE_NAME);
        lua_settop(g_mud_LS, 0);
        return TRUE;
    }
    lua_pushvalue( g_mud_LS, -2);
    lua_remove( g_mud_LS, -3);
    lua_gettable( g_mud_LS, -2);
    lua_remove( g_mud_LS, -2); /* don't need envtbl anymore*/
    if ( lua_isnil( g_mud_LS, -1) )
    {
        bugf("Game object not found for interpreter session for %s.", d->character->name);
        ptc( d->character, "Couldn't find game object, was it destroyed?\n\r"
                "Exiting interpreter.\n\r");
        d->lua.interpret=FALSE;

        ptc(d->character, "Exited lua interpreter.\n\r");
        lua_settop(g_mud_LS, 0);
        return TRUE;
    }

    /* if we're here then we just need to push the string and call the func */
    lua_pushstring( g_mud_LS, d->incomm);

    g_ScriptSecurity= d->character->pcdata->security ;
    g_LoopCheckCounter=0;
    g_LuaScriptInProgress = TRUE;

    int error=CallLuaWithTraceBack (g_mud_LS, 2, 1) ;
    if (error > 0 )
    {
        ptc(d->character,  "LUA error for lua_interpret:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        d->lua.incmpl=FALSE; //force it whether it was or wasn't
    } 
    else
    {
        bool incmpl=(bool)luaL_checknumber( g_mud_LS, -1 );
        if (incmpl)
        {
            d->lua.incmpl=TRUE;
        } 
        else
        {
            d->lua.incmpl=FALSE;
        
        }
    }

    g_ScriptSecurity = 0;
    g_LuaScriptInProgress=FALSE;

    lua_settop( g_mud_LS, 0);
    return TRUE;

}

void lua_unregister_desc (DESCRIPTOR_DATA *d)
{
    lua_getglobal( g_mud_LS, "UnregisterDesc");
    lua_pushlightuserdata( g_mud_LS, d);
    int error=CallLuaWithTraceBack (g_mud_LS, 1, 0) ;
    if (error > 0 )
    {
        ptc(d->character,  "LUA error for UnregisterDesc:\n %s",
                lua_tostring(g_mud_LS, -1));
    }
}

void do_luai( CHAR_DATA *ch, char *argument)
{
    if IS_NPC(ch)
        return;
    
    if ( ch->desc->lua.interpret )
    {
        ptc(ch, "Lua interpreter already active!\n\r");
        return;
    }

    char arg1[MSL];
    char *name;

    argument=one_argument(argument, arg1);

    void *victim=NULL;
    LUA_OBJ_TYPE *type;

    if ( arg1[0]== '\0' )
    {
        victim=(void *)ch;
        type=&CH_type;
        name=ch->name;
    }
    else if (!strcmp( arg1, "mob") )
    {
        CHAR_DATA *mob;
        mob=get_char_room( ch, argument );
        if (!mob)
        {
            ptc(ch, "Could not find %s in the room.\n\r", argument);
            return;
        }
        else if (!IS_NPC(mob))
        {
            ptc(ch, "Not on PCs.\n\r");
            return;
        }

        victim = (void *)mob;
        type= &CH_type;
        name=mob->name;
    }
    else if (!strcmp( arg1, "obj") )
    {
        OBJ_DATA *obj=NULL;
        obj=get_obj_here( ch, argument);

        if (!obj)
        {
            ptc(ch, "Could not find %s in room or inventory.\n\r", argument);
            return;
        }

        victim= (void *)obj;
        type=&OBJ_type;
        name=obj->name;
    }
    else if (!strcmp( arg1, "area") )
    {
        if (!ch->in_room)
        {
            bugf("do_luai: %s in_room is NULL.", ch->name);
            return;
        }

        victim= (void *)(ch->in_room->area);
        type=&AREA_type;
        name=ch->in_room->area->name;
    }
    else if (!strcmp( arg1, "room") )
    {
        if (!ch->in_room)
        {
            bugf("do_luai: %s in_room is NULL.", ch->name);
            return;
        }
        
        victim= (void *)(ch->in_room);
        type=&ROOM_type;
        name=ch->in_room->name;
    }
    else
    {
        ptc(ch, "luai [no argument] -- open interpreter in your own env\n\r"
                "luai mob <target>  -- open interpreter in env of target mob (in same room)\n\r"
                "luai obj <target>  -- open interpreter in env of target obj (inventory or same room)\n\r"
                "luai area          -- open interpreter in env of current area\n\r"
                "luai room          -- open interpreter in env of current room\n\r"); 
        return;
    }

    if (!ch->desc)
    {
        bugf("do_luai: %s has null desc", ch->name);
        return;
    }

    /* do the stuff */
    lua_getglobal( g_mud_LS, "interp_setup");
    if ( !type->push(g_mud_LS, victim) )
    {
        bugf("do_luai: couldn't make udtable argument %s",
                argument);
        lua_settop(g_mud_LS, 0);
        return;
    }

    if ( type == &CH_type )
    {
        lua_pushliteral( g_mud_LS, "mob"); 
    }
    else if ( type == &OBJ_type )
    {
        lua_pushliteral( g_mud_LS, "obj"); 
    }
    else if ( type == &AREA_type )
    {
        lua_pushliteral( g_mud_LS, "area"); 
    }
    else if ( type == &ROOM_type )
    {
        lua_pushliteral( g_mud_LS, "room"); 
    }
    else
    {
            bugf("do_luai: invalid type: %s", type->type_name);
            lua_settop(g_mud_LS, 0);
            return;
    }

    lua_pushlightuserdata(g_mud_LS, ch->desc);
    lua_pushstring( g_mud_LS, ch->name );

    int error=CallLuaWithTraceBack (g_mud_LS, 4, 2) ;
    if (error > 0 )
    {
        bugf ( "LUA error for interp_setup:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_settop(g_mud_LS, 0);
        return;
    }

    /* 2 values, true or false (false if somebody already interpreting on that object)
       and name of person interping if false */
    bool success=(bool)luaL_checknumber( g_mud_LS, -2);
    if (!success)
    {
        ptc(ch, "Can't open lua interpreter, %s already has it open for that object.\n\r",
                check_string( g_mud_LS, -1, MIL));
        lua_settop(g_mud_LS, 0);
        return;
    }

    /* finally, if everything worked out, we can set this stuff */
    ch->desc->lua.interpret=TRUE;
    ch->desc->lua.incmpl=FALSE;

    ptc(ch, "Entered lua interpreter mode for %s %s\n\r", 
            type->type_name,
            name);
    ptc(ch, "Use @ on a blank line to exit.\n\r");
    ptc(ch, "Use do and end to create multiline chunks.\n\r");
    ptc(ch, "Use '%s' to access target's self.\n\r",
            type == &CH_type ? "mob" :
            type == &OBJ_type ? "obj" :
            type == &ROOM_type ? "room" :
            type == &AREA_type ? "area" :
            "ERROR" );

    lua_settop(g_mud_LS, 0);
    return;
}

static int RegisterLuaRoutines (lua_State *LS)
{
    time_t timer;
    time (&timer);

    init_genrand (timer);
    type_init( LS );

    register_globals ( LS );

    return 0;

}  /* end of RegisterLuaRoutines */

void open_lua ()
{
    lua_State *LS = luaL_newstate ();   /* opens Lua */
    g_mud_LS = LS;

    if (g_mud_LS == NULL)
    {
        bugf("Cannot open Lua state");
        return;  /* catastrophic failure */
    }

    luaL_openlibs (LS);    /* open all standard libraries */

    /* call as Lua function because we need the environment  */
    lua_pushcfunction(LS, RegisterLuaRoutines);
    lua_call(LS, 0, 0);

    lua_sethook(LS, infinite_loop_check_hook, LUA_MASKCOUNT, LUA_LOOP_CHECK_INCREMENT);
    /* run initialiation script */
    if (luaL_loadfile (LS, LUA_STARTUP) ||
            CallLuaWithTraceBack (LS, 0, 0))
    {
        bugf ( "Error loading Lua startup file %s:\n %s", 
                LUA_STARTUP,
                lua_tostring(LS, -1));
    }

    lua_settop (LS, 0);    /* get rid of stuff lying around */

}  /* end of open_lua */

void do_scriptdump( CHAR_DATA *ch, char *argument )
{
    lua_getglobal(g_mud_LS, "do_scriptdump");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_scriptdump:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }

}
static int L_wizhelp( LS )
{
    CHAR_DATA *ud_ch=check_CH(LS, 1);
    
    int index=1; /* the current table index */
    lua_newtable( LS );/* the table of commands */
    
    int cmd;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level >= LEVEL_HERO
            &&  is_granted( ud_ch, cmd_table[cmd].do_fun )
            &&  cmd_table[cmd].show )
        {
            lua_newtable( LS ); /* this command's table */
            
            lua_pushinteger( LS, cmd_table[cmd].level );
            lua_setfield( LS, -2, "level" );

            lua_pushstring( LS, cmd_table[cmd].name );
            lua_setfield( LS, -2, "name" );
            
            lua_rawseti( LS, -2, index++ ); /* save the command to the table */
        }
    }

    lua_getglobal( LS, "wizhelp" );
    lua_insert( LS, 1 ); /* shove it to the top */

    lua_call( LS, lua_gettop(LS)-1, 0 );

    return 0;
}
     
void do_luaquery( CHAR_DATA *ch, char *argument)
{
    lua_getglobal( g_mud_LS, "do_luaquery");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_luaquery:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}    

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    lua_pushcfunction(g_mud_LS, L_wizhelp);
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_wizhelp:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

static CHAR_DATA *clt_list;
static int L_charloadtest( lua_State *LS )
{
    /* 1 is script function */
    /* 2 is ch */
    DESCRIPTOR_DATA d;
    clt_list=NULL;

    lua_getglobal( LS, "list_files");
    lua_pushliteral( LS, "../area");
    lua_call( LS, 1, 1 );
  
    lua_newtable( LS ); /* the table of chars */ 
    /* iterate the table */
    int index;
    int newindex=1;
    for ( index=1 ; ; index++ )
    {
        lua_rawgeti( LS, -2, index );
        if (lua_isnil( LS, -1) )
        {
            lua_pop( LS, 1 );
            break;
        }
        if (!load_char_obj( &d, check_string( LS, -1, MIL ) ) )
        {
            lua_pop( LS, 1 );
            continue;
            //luaL_error( LS, "Couldn't load '%s'", check_string( LS, -1, MIL) );
        }
        lua_pop( LS, 1 );

        d.character->next=clt_list;
        clt_list=d.character;

        d.character->desc=NULL;
        reset_char(d.character);
        if (!push_CH(LS, d.character) )
            luaL_error( LS, "Couldn't make UD for %s", d.character->name);
        lua_rawseti( LS, -2, newindex++);
    }
    
    lua_remove( LS, -2 ); /* kill name table */
    stackDump(LS);

    lua_call( LS, 2, 0 );
    return 0;

}
        
void do_charloadtest( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL];
    char arg2[MIL];
    const char *code=NULL;

    argument=one_argument(argument, arg1);
    argument=one_argument(argument, arg2);

    if (is_number(arg1))
    {
        PROG_CODE *prg=get_mprog_index( atoi(arg1));
        if (!prg)
        {
            ptc( ch, "No such mprog: %s\n\r", arg1 );
            return;
        }
        code=prg->code;
    }
    else
    {
        ptc(ch, "'%s' is not a valid mprog vnum.\n\r", arg1 );
        return;
    }

    char buf[MAX_SCRIPT_LENGTH + MSL ];

    sprintf(buf, "return function (ch, players)\n"
            "%s\n"
            "end",
            code);

    if (luaL_loadstring ( g_mud_LS, buf) ||
            CallLuaWithTraceBack ( g_mud_LS, 0, 1) )
    {
        ptc( ch, "Error loading vnum %s: %s",
                arg1,
                lua_tostring( g_mud_LS, -1));
        lua_settop( g_mud_LS, 0);
        return;
    }

    push_CH( g_mud_LS, ch);
    lua_pushcfunction(g_mud_LS, L_charloadtest);
    lua_insert( g_mud_LS, 1);

    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_charloadtest:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }

    /* clean up the stuff */
    CHAR_DATA *tch, *next;
    for ( tch=clt_list ; tch ; tch = next )
    {
        next=tch->next;
        free_char( tch );
    }

    clt_list=NULL;

    return;
}

const char *save_luaconfig( CHAR_DATA *ch )
{
    if (!IS_IMMORTAL(ch))
        return NULL;

    lua_getglobal(g_mud_LS, "save_luaconfig" );
    push_CH(g_mud_LS, ch);
    if (CallLuaWithTraceBack( g_mud_LS, 1, 1 ) )
    {
        ptc (ch, "Error with save_luaconfig:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
        return NULL;
    }

    const char *rtn;
    if (lua_isnil(g_mud_LS, -1) || lua_isnone(g_mud_LS, -1) )
    {
        rtn=NULL;
    }
    else if (!lua_isstring(g_mud_LS, -1))
    {
        bugf("String wasn't returned in save_luaconfig.");
        rtn=NULL;
    }
    else
    {
        rtn=luaL_checkstring( g_mud_LS, -1 );
    }

    lua_pop( g_mud_LS, 1 );

    return rtn;
}

void load_luaconfig( CHAR_DATA *ch, const char *text )
{
    lua_getglobal(g_mud_LS, "load_luaconfig" );
    push_CH(g_mud_LS, ch);
    lua_pushstring( g_mud_LS, text );

    if (CallLuaWithTraceBack( g_mud_LS, 2, 0 ) )
    {
        ptc (ch, "Error with load_luaconfig:\n %s",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
        return NULL;
    }
}

void do_luaconfig( CHAR_DATA *ch, char *argument)
{
    lua_getglobal(g_mud_LS, "do_luaconfig");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_luaconfig:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

static int L_dump_prog( lua_State *LS)
{
    // 1 is ch
    // 2 is prog 
    // 3 is numberlines
    bool numberlines=lua_toboolean(LS, 3);
    lua_pop(LS,1);


    lua_getglobal( LS, "colorize");
    lua_insert( LS, -2 );
    lua_pushvalue(LS,1); //push a copy of ch
    lua_call( LS, 2, 1 );

    // 1 is ch
    // 2 is colorized text
    if (numberlines)
    {
        lua_getglobal( LS, "linenumber");
        lua_insert( LS, -2);
        lua_call(LS, 1, 1);
    }
    lua_pushstring(LS, "\n\r");
    lua_concat( LS, 2);
    page_to_char_new( 
            luaL_checkstring(LS, 2),
            check_CH(LS, 1),
            TRUE);

    return 0;
}

void dump_prog( CHAR_DATA *ch, const char *prog, bool numberlines)
{
    lua_pushcfunction( g_mud_LS, L_dump_prog);
    push_CH(g_mud_LS, ch);
    lua_pushstring( g_mud_LS, prog);
    lua_pushboolean( g_mud_LS, numberlines);

    if (CallLuaWithTraceBack( g_mud_LS, 3, 0) )
    {
        ptc (ch, "Error with dump_prog:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void do_luareset( CHAR_DATA *ch, char *argument)
{
    lua_getglobal(g_mud_LS, "do_luareset");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_luareset:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void do_alist(CHAR_DATA *ch, char *argument)
{
    lua_getglobal(g_mud_LS, "do_alist");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_alist:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void do_mudconfig( CHAR_DATA *ch, char *argument)
{
    lua_getglobal(g_mud_LS, "do_mudconfig");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_mudconfig:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void do_perfmon( CHAR_DATA *ch, char *argument)
{
    lua_getglobal(g_mud_LS, "do_perfmon");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_perfmon:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void lua_log_perf( double value )
{
    lua_getglobal( g_mud_LS, "log_perf" );
    lua_pushnumber( g_mud_LS, value );
    lua_call( g_mud_LS, 1, 0 );
}

void do_findreset( CHAR_DATA *ch, char *argument)
{
    lua_getglobal(g_mud_LS, "do_findreset");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_findreset:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void do_diagnostic( CHAR_DATA *ch, char *argument)
{
    lua_getglobal(g_mud_LS, "do_diagnostic");
    push_CH(g_mud_LS, ch);
    lua_pushstring(g_mud_LS, argument);
    if (CallLuaWithTraceBack( g_mud_LS, 2, 0) )
    {
        ptc (ch, "Error with do_diagnostic:\n %s\n\r",
                lua_tostring(g_mud_LS, -1));
        lua_pop( g_mud_LS, 1);
    }
}

void check_lua_stack()
{
    int top=lua_gettop( g_mud_LS );
    if ( top > 0 )
    {
        bugf("%d items left on Lua stack. Clearing.", top );
        lua_settop( g_mud_LS, 0);
    }
}

